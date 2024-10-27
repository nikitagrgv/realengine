#include "WorkerThread.h"

#include "EngineGlobals.h"
#include "Job.h"
#include "JobQueue.h"
#include "Threads.h"

void WorkerThread::execute()
{
    while (true)
    {
        if (needExit())
        {
            break;
        }

        // TODO# condition variable

        UPtr<Job> job = eng.queue->takeJob();
        if (!job)
        {
            Threads::sleepMs(1);
            continue;
        }

        job->execute();
        job->finishWorkerThread();

        eng.queue->addFinishedJob(std::move(job));
    }
}