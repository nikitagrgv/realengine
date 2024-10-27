#pragma once
#include "Base.h"

class Job
{
public:
    REMOVE_COPY_MOVE_CLASS(Job);

    virtual ~Job() = default;

    virtual void execute() = 0;

    virtual void finish() {}
    virtual void finishMainThread() {}
};
