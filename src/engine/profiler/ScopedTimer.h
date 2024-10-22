#pragma once

class ScopedTimer
{
public:
    explicit ScopedTimer(const char *name = nullptr);
    ~ScopedTimer();

private:
    double begin_{-1};
    const char *name_ = nullptr;
};
