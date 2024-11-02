#include "Job.h"

#include <atomic>

tbb::CancelToken::CancelToken(std::shared_ptr<std::atomic<bool>> job_alive,
    std::shared_ptr<std::atomic<bool>> cancel_flag)
    : job_alive_(std::move(job_alive))
    , cancel_flag_(std::move(cancel_flag))
{
    assert(job_alive_);
    assert(cancel_flag_);
}

bool tbb::CancelToken::isCanceled() const
{
    return cancel_flag_->load();
}

void tbb::CancelToken::cancel()
{
    cancel_flag_->store(true);
}

bool tbb::CancelToken::isAlive() const
{
    return job_alive_->load();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

tbb::Job::Job()
    : job_alive_(std::make_shared<std::atomic<bool>>(true))
    , cancel_flag_(std::make_shared<std::atomic<bool>>(false))
{}

tbb::Job::~Job()
{
    job_alive_->store(false);
}

bool tbb::Job::isCanceled() const
{
    return cancel_flag_->load();
}

void tbb::Job::cancel()
{
    cancel_flag_->store(true);
}

tbb::CancelToken tbb::Job::getCancelToken() const
{
    return CancelToken(job_alive_, cancel_flag_);
}