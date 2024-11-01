#include "ScopedProfiler.h"

#include "EngineGlobals.h"
#include "fs/FileSystem.h"

#include <immintrin.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
#endif

#undef min
#undef max

namespace
{

uint64_t get_perf_counter();
uint64_t get_perf_frequency();

void dump_svg();
void dump_html();

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

ScopedProfiler::ScopedProfiler(const char *name)
{
    Profiler::enterFunction(name, get_perf_counter());
}

ScopedProfiler::~ScopedProfiler()
{
    Profiler::leaveFunction(get_perf_counter());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{

struct ProbeInfo
{
    ProbeInfo(const char *name, uint64_t time)
        : name(name)
        , time(time)
    {}

    const char *name; // valid - enterFunction, nullptr - leaveFunction
    uint64_t time;
};


enum class DumpType
{
    None,
    SVG,
    HTML,
};

DumpType REQUESTED_DUMP{DumpType::HTML};
std::string DUMP_PATH{};

uint64_t PERF_FREQ{};

int HALF_MAX_RECORDED_FRAMES{100};
int CUR_RECORDED_FRAMES{};

std::vector<ProbeInfo> OLD_PROBES;
std::vector<ProbeInfo> PROBES;

} // namespace

void Profiler::init()
{
    PERF_FREQ = get_perf_frequency();
}

void Profiler::setMaxRecordedFrames(int frames)
{
    // TODO# shitty but i don't care
    HALF_MAX_RECORDED_FRAMES = 1 + frames / 2;
}

void Profiler::enterFunction(const char *name, uint64_t time)
{
    PROBES.emplace_back(name, time);
}

void Profiler::leaveFunction(uint64_t time)
{
    PROBES.emplace_back(nullptr, time);
}

void Profiler::enterFunction(const char *name)
{
    enterFunction(name, get_perf_counter());
}

void Profiler::leaveFunction()
{
    leaveFunction(get_perf_counter());
}

void Profiler::beginFrame()
{
    enterFunction("Frame Total", get_perf_counter());
}

void Profiler::endFrame()
{
    leaveFunction(get_perf_counter());

    if (REQUESTED_DUMP == DumpType::SVG)
    {
        dump_svg();
        REQUESTED_DUMP = DumpType::None;
    }
    else if (REQUESTED_DUMP == DumpType::HTML)
    {
        dump_html();
        REQUESTED_DUMP = DumpType::None;
    }

    ++CUR_RECORDED_FRAMES;
    if (CUR_RECORDED_FRAMES > HALF_MAX_RECORDED_FRAMES)
    {
        CUR_RECORDED_FRAMES = 0;

        const int size = PROBES.size();
        const int capacity = PROBES.capacity();
        if (capacity < size * 2)
        {
            PROBES.reserve(size * 3);
            OLD_PROBES.reserve(size * 3);
        }

        // don't use std::move! otherwise a deallocation will happen
        std::swap(OLD_PROBES, PROBES);
        PROBES.clear();
    }
}

void Profiler::dumpSVG(const char *path)
{
    DUMP_PATH = path;
    REQUESTED_DUMP = DumpType::SVG;
}

void Profiler::dumpHTML(const char *path)
{
    DUMP_PATH = path;
    REQUESTED_DUMP = DumpType::HTML;
}

namespace
{

uint64_t get_perf_counter()
{
#ifdef _WIN32
    LARGE_INTEGER val;
    QueryPerformanceCounter(&val);
    return val.QuadPart;
#else
    static_assert(0, "Not implemented");
    return 0;
#endif
}

uint64_t get_perf_frequency()
{
#ifdef _WIN32
    LARGE_INTEGER val;
    QueryPerformanceFrequency(&val);
    return val.QuadPart;
#else
    static_assert(0, "Not implemented");
    return 0;
#endif
}

char TEMP_BUFFER[2048];

void dump_svg()
{
    if (OLD_PROBES.empty() && PROBES.empty())
    {
        return;
    }

    const uint64_t start_time = !OLD_PROBES.empty() ? OLD_PROBES[0].time : PROBES[0].time;
    const uint64_t end_time = !PROBES.empty() ? PROBES.back().time : OLD_PROBES.back().time;

    std::ofstream out(DUMP_PATH);

    int max_depth = 0;

    struct Block
    {
        const char *name = nullptr;
        uint64_t start = 0;
        uint64_t end = 0;
        int depth = -1;
    };
    std::vector<Block> stack;
    std::vector<Block> final_blocks;
    const auto add_probe = [&](const char *name, uint64_t time) {
        if (name)
        {
            Block block;
            block.name = name;
            block.start = time - start_time;
            block.depth = stack.size();
            if (block.depth > max_depth)
            {
                max_depth = block.depth;
            }
            stack.push_back(block);
        }
        else
        {
            assert(!stack.empty());
            Block &block = stack.back();
            block.end = time - start_time;
            final_blocks.push_back(block);
            stack.pop_back();
        }
    };

    for (const auto &probe : OLD_PROBES)
    {
        add_probe(probe.name, probe.time);
    }
    for (const auto &probe : PROBES)
    {
        add_probe(probe.name, probe.time);
    }

    // TODO: stupid but i don't care
    std::sort(final_blocks.begin(), final_blocks.end(), [](const Block &a, const Block &b) {
        if (a.start == b.start)
        {
            return a.depth < b.depth;
        }
        return a.start < b.start;
    });

    const double total_duration_ms = (end_time - start_time) * 1000 / PERF_FREQ;

    const double block_height = 50.0;
    const double half_block_height = block_height / 2;

    const double total_width = 5000;
    const double total_height = (double)(max_depth + 1) * block_height;

    const auto print_block = [&](const char *name, uint64_t start, uint64_t end, int depth) {
        const double start_ms = (double)start * 1000.0 / (double)PERF_FREQ;
        const double end_ms = (double)end * 1000.0 / (double)PERF_FREQ;
        const double duration_ms = (double)((end - start) * 1000.0) / (double)PERF_FREQ;

        const double x = total_width * start_ms / total_duration_ms;
        const double y = block_height * depth;
        const double width = total_width * duration_ms / total_duration_ms;
        const double half_width = width / 2;

        double stroke_width = 1.0f;
        if (width <= 4)
        {
            stroke_width = width / 10.0f;
        }

        sprintf(TEMP_BUFFER,
            R"!( <rect x="%lf" y="%lf" width="%lf" height="%lf" style="fill:lightblue;stroke:black;stroke-width:%lf">)!",
            x, y, width, block_height, stroke_width);
        out << TEMP_BUFFER << "\n";

        sprintf(TEMP_BUFFER, R"!(  <title>%s (%.3f ms)</title>)!", name, duration_ms);
        out << TEMP_BUFFER << "\n";

        out << "</rect>\n";

        const double font_size = std::min(width * 0.1, block_height - 1);
        sprintf(TEMP_BUFFER,
            R"!( <text x="%lf" y="%lf" font-family="Arial" font-size="%lfpx" fill="black" dominant-baseline="middle" text-anchor="middle">%s</text>)!",
            x + half_width, y + half_block_height, font_size, name);
        out << TEMP_BUFFER << "\n";
    };

    sprintf(TEMP_BUFFER, R"!(<svg viewBox="0 0 %lf %lf" xmlns="http://www.w3.org/2000/svg">)!",
        total_width, total_height);
    out << TEMP_BUFFER << "\n";

    for (const auto &block : final_blocks)
    {
        print_block(block.name, block.start, block.end, block.depth);
    }

    out << "</svg>\n";
}

void dump_html()
{
    if (OLD_PROBES.empty() && PROBES.empty())
    {
        return;
    }

    const uint64_t start_time = !OLD_PROBES.empty() ? OLD_PROBES[0].time : PROBES[0].time;
    const uint64_t end_time = !PROBES.empty() ? PROBES.back().time : OLD_PROBES.back().time;

    std::ofstream out(DUMP_PATH, std::ios::binary);

    int max_depth = 0;

    struct Block
    {
        const char *name = nullptr;
        uint64_t start = 0;
        uint64_t end = 0;
        int depth = -1;
    };
    std::vector<Block> stack;
    std::vector<Block> final_blocks;
    const auto add_probe = [&](const char *name, uint64_t time) {
        if (name)
        {
            Block block;
            block.name = name;
            block.start = time - start_time;
            block.depth = stack.size();
            if (block.depth > max_depth)
            {
                max_depth = block.depth;
            }
            stack.push_back(block);
        }
        else
        {
            assert(!stack.empty());
            Block &block = stack.back();
            block.end = time - start_time;
            final_blocks.push_back(block);
            stack.pop_back();
        }
    };

    for (const auto &probe : OLD_PROBES)
    {
        add_probe(probe.name, probe.time);
    }
    for (const auto &probe : PROBES)
    {
        add_probe(probe.name, probe.time);
    }

    // TODO: stupid but i don't care
    std::sort(final_blocks.begin(), final_blocks.end(), [](const Block &a, const Block &b) {
        if (a.start == b.start)
        {
            return a.depth < b.depth;
        }
        return a.start < b.start;
    });

    const double total_duration_ms = (end_time - start_time) * 1000 / PERF_FREQ;

    const double block_height = 50.0;
    const double half_block_height = block_height / 2;

    const double total_width = 5000;
    const double total_height = (double)(max_depth + 1) * block_height;

    const auto print_block = [&](const char *name, uint64_t start, uint64_t end, int depth) {
        const double start_ms = (double)start * 1000.0 / (double)PERF_FREQ;
        const double duration_ms = (double)((end - start) * 1000.0) / (double)PERF_FREQ;
        sprintf(TEMP_BUFFER, "{label: '%s', start: %lf, duration: %lf},", name, start_ms,
            duration_ms);
        out << TEMP_BUFFER << "\n";
    };

    const std::string template_path = eng.fs->toAbsolutePath("base/profiler.html");
    const std::string template_content = eng.fs->readFile(template_path.c_str());

    const char *placelolder = "// $DATA_PLACEHOLDER$";
    const size_t placeholder_index = template_content.find(placelolder);

    if (placeholder_index == -1)
    {
        std::cout << "Invalid profilre HTML template" << std::endl;
        return;
    }
    const size_t after_placeholder = placeholder_index + strlen(placelolder);
    out.write(template_content.data(), placeholder_index);

    for (const auto &block : final_blocks)
    {
        print_block(block.name, block.start, block.end, block.depth);
    }

    out.write(template_content.data() + after_placeholder,
        template_content.size() - after_placeholder);
}


} // namespace
