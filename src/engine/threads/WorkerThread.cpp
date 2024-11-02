#include "WorkerThread.h"

#include "EngineGlobals.h"
#include "Job.h"
#include "JobQueue.h"
#include "Threads.h"
#include "profiler/ScopedProfiler.h"

namespace tbb
{

void WorkerThread::execute()
{
    set_state(State::Idle);
    while (true)
    {
        if (needExit())
        {
            break;
        }

        UPtr<Job> job = eng.queue->takeJobWaiting(*this);
        if (!job)
        {
            continue;
        }

        ScopedProfiler prof("WorkerThread::Execute job");
        set_state(State::Working);
        job->execute();
        job->finishWorkerThread();

        eng.queue->addFinishedJob(std::move(job));
        set_state(State::Idle);
    }
}

WorkerThread::State WorkerThread::getState() const
{
    const int state_value = state_.load();
    const State state = static_cast<State>(state_value);
    assert(state == State::Idle || state == State::Working);
    return state;
}

void WorkerThread::set_state(State state)
{
    assert(state == State::Idle || state == State::Working);
    const int state_value = static_cast<int>(state);
    state_.store(state_value);
}

} // namespace tbb
