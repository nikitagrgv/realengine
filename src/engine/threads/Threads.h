#pragma once

#include <cstdint>

class Threads
{
public:
    static void init();
    static void shutdown();

    static bool isMainThread();

    static uint64_t getCurrentThreadId();

    static bool getMainThreadId();

    static void sleepMs(uint64_t ms);

private:
    static uint64_t main_thread_;
};
