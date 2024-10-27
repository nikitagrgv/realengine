#include "Thread.h"

#include <windows.h>

namespace
{

DWORD WINAPI run_thread(LPVOID param)
{
    auto thread = static_cast<Thread *>(param);
    thread->execute();
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