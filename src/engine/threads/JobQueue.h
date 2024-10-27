#pragma once

#include "Base.h"

#include <queue>

#include <mutex>
#include <vector>

class Job;
class WorkerThread;

class JobQueue
{
public:
    REMOVE_COPY_MOVE_CLASS(JobQueue);

    JobQueue();
    ~JobQueue();

    void runWorkers();

    void finishJobsMainThread();

    void enqueueJob(UPtr<Job> job);

    void addFinishedJob(UPtr<Job> job);

    UPtr<Job> takeJob();
    int getNumJobs();

private:
    int num_threads_{};
    std::vector<UPtr<WorkerThread>> threads_;

    std::mutex jobs_mutex_;
    std::queue<UPtr<Job>> jobs_;

    std::mutex finished_mutex_;
    std::queue<UPtr<Job>> finished_jobs_;
};
