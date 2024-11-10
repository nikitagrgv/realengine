#include "Thread.h"

#include <windows.h>

namespace
{

DWORD WINAPI run_thread(LPVOID param)
{
    auto thread = static_cast<Thread *>(param);
    thread->run();
    return 0;
}

} // namespace

Thread::Thread()
{
    DWORD thread_id;
    handle_ = CreateThread(nullptr, // Default security attributes
        0,                          // Default stack size
        &run_thread,                // Thread function
        this,                       // Thread parameters
        0,                          // Default creation flags
        &thread_id                  // Receive thread ID
    );
    id_ = thread_id;
}

Thread::~Thread()
{
    exit();
    join();
}

void Thread::join()
{
    if (handle_)
    {
        WaitForSingleObject(handle_, INFINITE);
        CloseHandle(handle_);
        handle_ = nullptr;
        id_ = 0;
    }
}

void Thread::exit()
{
    exit_.store(true);
}

void Thread::kill()
{
    TerminateThread(handle_, 0);
    finished_.store(true);
}

bool Thread::needExit() const
{
    return exit_.load();
}

void Thread::run()
{
    finished_.store(false);
    execute();
    finished_.store(true);
}

bool Thread::isFinished() const
{
    return finished_.load();
}

uint64_t Thread::getId() const
{
    return id_;
}
