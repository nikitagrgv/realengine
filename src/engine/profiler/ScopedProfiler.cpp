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

void Profiler::endFrame()
{
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
    std::ofstream out(path);
    out << "<svg width=\"100%\" height=\"100%\" viewBox=\"0 0 100 100\" "
           "xmlns=\"http://www.w3.org/2000/svg\">\n";
    for (const auto &probe : OLD_PROBES)
    {
        if (probe.name)
        {
            out << "\t<rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" fill=\"red\"/>\n";
        }
        else
        {
            out << "\t<rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" fill=\"blue\"/>\n";
        }
    }
    for (const auto &probe : PROBES)
    {
        if (probe.name)
        {
            out << "\t<rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" fill=\"red\"/>\n";
        }
        else
        {
            out << "\t<rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" fill=\"blue\"/>\n";
        }
    }
    out << "</svg>\n";
}