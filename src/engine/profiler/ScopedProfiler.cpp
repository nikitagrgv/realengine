#include "ScopedProfiler.h"

#include <immintrin.h>

#include <cassert>
#include <cstdint>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace
{

#ifdef _WIN32
LARGE_INTEGER PERF_FREQ{};
#endif

} // namespace

namespace
{

// TODO# don't divide here. do it in statistic collection stage
uint64_t get_time_usec()
{
#ifdef _WIN32
    LARGE_INTEGER tval;
    QueryPerformanceCounter(&tval);

    unsigned long long counter_high_part = 0;
    unsigned long long counter = tval.QuadPart;

    counter = _umul128(counter, 1'000'000, &counter_high_part);
    #if _MSC_VER >= 1920
    return _udiv128(counter_high_part, counter, PERF_FREQ.QuadPart, nullptr);
    #endif
#else
    // TODO#
    return 0;
#endif
}

} // namespace


ScopedProfiler::ScopedProfiler(const char *name)
    : start_(get_time_usec())
{}

ScopedProfiler::~ScopedProfiler()
{
    const uint64_t end = get_time_usec();
    todo
}