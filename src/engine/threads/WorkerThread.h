#pragma once

#include "Thread.h"

namespace tbb
{

class WorkerThread : public Thread
{
public:
    enum class State
    {
        Idle,
        Busy,
    };

public:
    void execute() override;

    State getState() const;

private:
    void set_state(State state);

private:
    std::atomic<int> state_;
};

} // namespace tbb
