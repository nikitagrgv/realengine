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

        job->execute();
        job->finishWorkerThread();

        eng.queue->addFinishedJob(std::move(job));
    }
}

} // namespace tbb
