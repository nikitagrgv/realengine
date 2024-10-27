#include "JobQueue.h"

#include "Job.h"
#include "Threads.h"
#include "WorkerThread.h"
#include "profiler/ScopedProfiler.h"

#include <iostream>
#include <thread>

namespace tbb
{

JobQueue::JobQueue()
{
    num_threads_ = std::thread::hardware_concurrency();
    if (num_threads_ <= 0)
    {
        num_threads_ = 1;
    }
    std::cout << "JobQueue: initialized with " << num_threads_ << " threads" << std::endl;
}

JobQueue::~JobQueue()
{
    threads_.clear();

    std::queue<UPtr<Job>> taken_jobs;
    {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        std::swap(taken_jobs, jobs_);
    }

    const int num_jobs = taken_jobs.size();
    while (!taken_jobs.empty())
    {
        taken_jobs.pop();
    }

    std::cout << "JobQueue: unfinished jobs " << num_jobs << std::endl;
}

void JobQueue::runWorkers()
{
    assert(threads_.empty());
    for (int i = 0; i < num_threads_; ++i)
    {
        threads_.push_back(makeU<WorkerThread>());
    }
}

void JobQueue::finishJobsMainThread()
{
    assert(Threads::isMainThread());

    SCOPED_PROFILER;

    std::queue<UPtr<Job>> taken_jobs;
    {
        std::lock_guard<std::mutex> lock(finished_mutex_);
        std::swap(taken_jobs, finished_jobs_);
    }

    while (!taken_jobs.empty())
    {
        UPtr<Job> job = std::move(taken_jobs.front());
        taken_jobs.pop();
        job->finishMainThread();
    }
}

void JobQueue::enqueueJob(UPtr<Job> job)
{
    {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        jobs_.push(std::move(job));
    }
}

void JobQueue::addFinishedJob(UPtr<Job> job)
{
    std::lock_guard<std::mutex> lock(finished_mutex_);
    finished_jobs_.push(std::move(job));
}

UPtr<Job> JobQueue::takeJob()
{
    UPtr<Job> job;
    {
        std::lock_guard<std::mutex> lock(jobs_mutex_);
        if (!jobs_.empty())
        {
            job = std::move(jobs_.front());
            jobs_.pop();
        }
    }
    return job;
}

int JobQueue::getNumJobs()
{
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    return jobs_.size();
}

} // namespace tbb
