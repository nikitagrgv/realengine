#include "ScopedTimer.h"

#include "EngineGlobals.h"
#include "time/Time.h"

#include <iostream>

ScopedTimer::ScopedTimer(const char *name)
    : name_(name)
{
    begin_ = eng.time->getTime();
}

ScopedTimer::~ScopedTimer()
{
    const double end = eng.time->getTime();

    double duration = end - begin_;

    const char *m = "sec";
    if (duration < 1)
    {
        duration *= 1000.0;
        m = "ms";
        if (duration < 1)
        {
            duration *= 1000.0;
            m = "us";
            if (duration < 1)
            {
                duration *= 1000.0;
                m = "ns";
            }
        }
    }

    if (name_)
    {
        printf("Timer (%s) = %.1f%s\n", name_, duration, m);
    }
    else
    {
        printf("Timer = %.1f%s\n", duration, m);
    }
    fflush(stdout);
}