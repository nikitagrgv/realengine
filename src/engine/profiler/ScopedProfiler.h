#pragma once

#include <cstdint>

class ScopedProfiler
{
public:
    explicit ScopedProfiler(const char *name);
    ~ScopedProfiler();

private:
    uint64_t start_;
};
