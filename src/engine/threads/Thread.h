#pragma once

#include <cstdint>

class Thread
{
public:
    static void init();
    static void shutdown();

    static bool isMainThread();

    static uint64_t getCurrentThreadId();

private:
    static uint64_t main_thread_;
};
