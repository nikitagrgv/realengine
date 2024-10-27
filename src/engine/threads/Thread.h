#pragma once

#include "Base.h"

#include <cstdint>

class Thread
{
public:
    REMOVE_COPY_MOVE_CLASS(Thread);

    Thread();
    ~Thread();

    void join();

    virtual void execute() = 0;

private:
    void *handle_{};
    uint64_t id_{};
};
