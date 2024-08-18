#include "Time.h"

#include <immintrin.h>

#ifdef _WIN32
    #include <Windows.h>

    #include <cassert>
#endif

namespace
{

#ifdef _WIN32
LARGE_INTEGER PERF_FREQ{};
#endif

} // namespace

uint64_t Time::getTimeUsec() const
{
#ifdef _WIN32
    LARGE_INTEGER tval;
    QueryPerformanceCounter(&tval);

    unsigned long long counter_high_part = 0;
    unsigned long long counter = (tval.QuadPart - start_time_usec);

    counter = _umul128(counter, 1'000'000, &counter_high_part);
    #if _MSC_VER >= 1920
    return _udiv128(counter_high_part, counter, PERF_FREQ.QuadPart, nullptr);
    #endif
#else
    struct timeval tval;
    gettimeofday(&tval, nullptr);
    return (uint64_t)tval.tv_sec * 1000000 + tval.tv_usec - start_time_usec;
#endif
}

double Time::getTime() const
{
    return (double)getTimeUsec() / 1'000'000.0;
}

Time::Time()
{
#ifdef _WIN32
    QueryPerformanceFrequency(&PERF_FREQ);
    assert(PERF_FREQ.QuadPart <= UINT_MAX);
#endif

    start_time_usec = getTimeUsec();
    update();
    frame_ = 0; // TODO: shit?
}

Time::~Time() {}

void Time::update()
{
    ++frame_;

    old_time_usec = cur_time_usec;
    cur_time_usec = getTimeUsec();

    cur_time_ = (double)cur_time_usec / 1000000.0;

    delta_ms_ = double(cur_time_usec - old_time_usec) / 1000.0;
    delta_sec_ = double(cur_time_usec - old_time_usec) / 1000000.0;

    fps_ = 1.0 / delta_sec_;
}
