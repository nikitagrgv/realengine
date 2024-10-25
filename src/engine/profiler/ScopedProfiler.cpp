#include "ScopedProfiler.h"

#include <immintrin.h>

#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
#endif

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
    enterFunction("BEGIN FRAME", get_perf_counter());
}

void Profiler::endFrame()
{
    leaveFunction(get_perf_counter());

    ++CUR_RECORDED_FRAMES;
    if (CUR_RECORDED_FRAMES > HALF_MAX_RECORDED_FRAMES)
    {
        CUR_RECORDED_FRAMES = 0;

        // don't use std::move! otherwise a deallocation will happen
        std::swap(OLD_PROBES, PROBES);
        PROBES.clear();
    }
}

char TEMP_BUFFER[2048];
void Profiler::dumpSVG(const char *path)
{
    if (OLD_PROBES.empty() && PROBES.empty())
    {
        return;
    }

    const uint64_t begin_time = !OLD_PROBES.empty() ? OLD_PROBES[0].time : PROBES[0].time;

    std::ofstream out(path);

    const auto print_block = [&](const char *name, uint64_t start, uint64_t end, int depth) {
        while (depth)
        {
            --depth;
            out << " ";
        }
        sprintf(TEMP_BUFFER, "%s , %lld - %lld\n", name, start - begin_time, end - begin_time);
        out << TEMP_BUFFER;
    };

    struct Block
    {
        const char *name;
        uint64_t start;
        uint64_t end;
    };
    std::vector<Block> stack;
    const auto add_probe = [&](const char *name, uint64_t time) {
        if (name)
        {
            Block block;
            block.name = name;
            block.start = time;
            stack.push_back(block);
        }
        else
        {
            assert(!stack.empty());
            Block &block = stack.back();
            block.end = time;
            print_block(block.name, block.start, block.end, stack.size() - 1);
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

    const int total_width = 100;
    const int total_height = 100;
    sprintf(TEMP_BUFFER,
        R"!(<svg width="100%%" height="100%%" viewBox="0 0 %d %d" xmlns="http://www.w3.org/2000/svg">)!",
        total_width, total_height);
    out << TEMP_BUFFER << "\n";

    out << "</svg>\n";
}