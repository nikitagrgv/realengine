#pragma once

#include "Thread.h"

class WorkerThread : public Thread
{
public:
    void execute() override;
};
