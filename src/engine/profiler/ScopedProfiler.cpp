#include "ScopedProfiler.h"

#include <immintrin.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace
{

uint64_t get_perf_counter();
uint64_t get_perf_frequency();

void dump_svg();

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

bool DUMP_REQUESTED{false};
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

    if (DUMP_REQUESTED)
    {
        dump_svg();
    }

    ++CUR_RECORDED_FRAMES;
    if (CUR_RECORDED_FRAMES > HALF_MAX_RECORDED_FRAMES)
    {
        CUR_RECORDED_FRAMES = 0;

        // don't use std::move! otherwise a deallocation will happen
        std::swap(OLD_PROBES, PROBES);
        PROBES.clear();
    }
}

void Profiler::dumpSVG(const char *path)
{
    DUMP_PATH = path;
    DUMP_REQUESTED = true;
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
    DUMP_REQUESTED = false;

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

    const double total_width = 5000;
    const double total_height = 500;

    const double block_height = total_height / max_depth;
    const int font_size = (int)(block_height * 0.4);

    const auto print_block = [&](const char *name, uint64_t start, uint64_t end, int depth) {
        const double start_ms = start * 1000 / PERF_FREQ;
        const double end_ms = end * 1000 / PERF_FREQ;
        const double duration_ms = (end - start) * 1000 / PERF_FREQ;

        const double x = total_width * start_ms / total_duration_ms;
        const double y = total_height * depth / max_depth;
        const double width = total_width * duration_ms / total_duration_ms;

        sprintf(TEMP_BUFFER,
            R"!( <rect x="%lf" y="%lf" width="%lf" height="%lf" style="fill:lightblue;stroke:black;stroke-width:1"/>)!",
            x, y, width, block_height);
        out << TEMP_BUFFER << "\n";

        sprintf(TEMP_BUFFER,
            R"!( <text x="%lf" y="%lf" font-family="Arial" font-size="%dpx" fill="black" dominant-baseline="middle" text-anchor="middle">%s</text>)!",
            (x + width) / 2.0, (y + block_height) / 2.0, font_size, name);
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

} // namespace
