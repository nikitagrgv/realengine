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
    const double duration = end - begin_;
    if (name_)
    {
        printf("Timer (%s) = %f\n", name_, duration);
    }
    else
    {
        printf("Timer = %f\n", duration);
    }
}