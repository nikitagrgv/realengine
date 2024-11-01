#pragma once

#include "Base.h"

#include <cstdint>

class ScopedProfiler
{
public:
    explicit ScopedProfiler(const char *name);
    ~ScopedProfiler();
};

class Profiler
{
public:
    static void init();

    static void setMaxRecordedFrames(int frames);

    static void enterFunction(const char *name, uint64_t time);
    static void leaveFunction(uint64_t time);

    static void enterFunction(const char *name);
    static void leaveFunction();

    static void beginFrame();
    static void endFrame();

    static void dumpSVG(const char *path);
    static void dumpHTML(const char *path);
};

#define SCOPED_PROFILER ScopedProfiler REALENGINE_CONCATENATE(_profiler_, __LINE__)(__FUNCTION__)