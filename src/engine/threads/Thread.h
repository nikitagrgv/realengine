#pragma once

#include "Base.h"

#include <atomic>
#include <cstdint>

class Thread
{
public:
    REMOVE_COPY_MOVE_CLASS(Thread);

    Thread();
    virtual ~Thread();

    void join();
    void exit();

    bool needExit() const;

    virtual void execute() = 0;

private:
    std::atomic<bool> exit_;
    void *handle_{};
    uint64_t id_{};
};
