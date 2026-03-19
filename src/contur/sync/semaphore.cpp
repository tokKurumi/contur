/// @file semaphore.cpp
/// @brief Semaphore implementation.

#include "contur/sync/semaphore.h"

#include <algorithm>
#include <deque>

namespace contur {

    struct Semaphore::Impl
    {
        std::size_t count = 0;
        std::size_t maxCount = 1;
        std::deque<ProcessId> waitQueue;
    };

    Semaphore::Semaphore(std::size_t initialCount, std::size_t maxCount)
        : impl_(std::make_unique<Impl>())
    {
        impl_->maxCount = std::max<std::size_t>(1, maxCount);
        impl_->count = std::min(initialCount, impl_->maxCount);
    }

    Semaphore::~Semaphore() = default;
    Semaphore::Semaphore(Semaphore &&) noexcept = default;
    Semaphore &Semaphore::operator=(Semaphore &&) noexcept = default;

    Result<void> Semaphore::acquire(ProcessId pid)
    {
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        if (impl_->count > 0)
        {
            --impl_->count;
            return Result<void>::ok();
        }

        if (std::find(impl_->waitQueue.begin(), impl_->waitQueue.end(), pid) == impl_->waitQueue.end())
        {
            impl_->waitQueue.push_back(pid);
        }
        return Result<void>::error(ErrorCode::ResourceBusy);
    }

    Result<void> Semaphore::release(ProcessId pid)
    {
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        if (!impl_->waitQueue.empty())
        {
            impl_->waitQueue.pop_front();
            return Result<void>::ok();
        }

        if (impl_->count >= impl_->maxCount)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        ++impl_->count;
        return Result<void>::ok();
    }

    Result<void> Semaphore::tryAcquire(ProcessId pid)
    {
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        if (impl_->count == 0)
        {
            return Result<void>::error(ErrorCode::ResourceBusy);
        }

        --impl_->count;
        return Result<void>::ok();
    }

    std::string_view Semaphore::name() const noexcept
    {
        return "Semaphore";
    }

    std::size_t Semaphore::count() const noexcept
    {
        return impl_->count;
    }

    std::size_t Semaphore::maxCount() const noexcept
    {
        return impl_->maxCount;
    }

    std::size_t Semaphore::waitingCount() const noexcept
    {
        return impl_->waitQueue.size();
    }

} // namespace contur
