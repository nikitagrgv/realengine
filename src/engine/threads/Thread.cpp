#include "Thread.h"

#include <windows.h>

#include <thread>

uint64_t Thread::main_thread_ = 0;

void Thread::init()
{
    main_thread_ = getCurrentThreadId();
}

void Thread::shutdown() {}

bool Thread::isMainThread()
{
    return GetCurrentThreadId() == main_thread_;
}

uint64_t Thread::getCurrentThreadId()
{
    return GetCurrentThreadId();
}