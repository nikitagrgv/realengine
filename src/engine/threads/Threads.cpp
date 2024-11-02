#include "Threads.h"

#include <windows.h>

#include <thread>

uint64_t Threads::main_thread_ = 0;

void Threads::init()
{
    main_thread_ = getCurrentThreadId();
}

void Threads::shutdown() {}

bool Threads::isMainThread()
{
    return GetCurrentThreadId() == main_thread_;
}

uint64_t Threads::getCurrentThreadId()
{
    return GetCurrentThreadId();
}

uint64_t Threads::getMainThreadId()
{
    return main_thread_;
}

void Threads::sleepMs(uint64_t ms)
{
    Sleep(ms);
}