#pragma once
#include "Base.h"

namespace tbb
{

class CancelToken
{
public:
    explicit CancelToken(std::shared_ptr<std::atomic<bool>> job_alive,
        std::shared_ptr<std::atomic<bool>> cancel_flag);

    bool isCanceled() const;
    void cancel();

    bool isAlive() const;

private:
    std::shared_ptr<std::atomic<bool>> job_alive_;
    std::shared_ptr<std::atomic<bool>> cancel_flag_;
};

class Job
{
public:
    REMOVE_COPY_MOVE_CLASS(Job);

    Job();
    virtual ~Job();

    virtual void execute() = 0;

    bool isCanceled() const;
    void cancel();
    CancelToken getCancelToken() const;

    virtual void finishWorkerThread() {}
    virtual void finishMainThread() {}

private:
    std::shared_ptr<std::atomic<bool>> job_alive_;
    std::shared_ptr<std::atomic<bool>> cancel_flag_;
};

} // namespace tbb