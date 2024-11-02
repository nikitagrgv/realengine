#pragma once

#include "Base.h"

#include <queue>

#include <mutex>
#include <vector>

namespace tbb
{

class Job;
class WorkerThread;

} // namespace tbb

namespace tbb
{


class JobQueue
{
public:
    REMOVE_COPY_MOVE_CLASS(JobQueue);

    JobQueue();
    ~JobQueue();

    void runWorkers();
    void stopWorkers();

    void finishJobsMainThread();

    void enqueueJob(UPtr<Job> job);

    void addFinishedJob(UPtr<Job> job);

    UPtr<Job> takeJobWaiting(const WorkerThread &thread);
    UPtr<Job> tryTakeJob();
    int getNumJobs();

private:
    int num_threads_{};
    std::vector<UPtr<WorkerThread>> threads_;

    std::mutex jobs_mutex_;
    std::queue<UPtr<Job>> jobs_;
    std::condition_variable jobs_cv_;

    std::mutex finished_mutex_;
    std::queue<UPtr<Job>> finished_jobs_;
};

} // namespace tbb
