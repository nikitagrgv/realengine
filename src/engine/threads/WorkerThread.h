#pragma once

#include "Thread.h"

namespace tbb
{

class WorkerThread : public Thread
{
public:
    void execute() override;
};

} // namespace tbb
