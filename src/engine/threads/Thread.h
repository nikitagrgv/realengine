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

    void kill();

    bool needExit() const;

    void run();

    virtual void execute() = 0;

    bool isFinished() const;

    uint64_t getId() const;

private:
    std::atomic<bool> exit_;
    void *handle_{};
    uint64_t id_{};

    std::atomic<bool> finished_;
};
