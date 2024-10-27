#pragma once
#include "Base.h"

namespace tbb
{

class Job
{
public:
    REMOVE_COPY_MOVE_CLASS(Job);

    Job() = default;
    virtual ~Job() = default;

    virtual void execute() = 0;

    virtual void finishWorkerThread() {}
    virtual void finishMainThread() {}
};

} // namespace tbb