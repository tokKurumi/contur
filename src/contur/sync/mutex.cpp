/// @file mutex.cpp
/// @brief Mutex implementation.

#include "contur/sync/mutex.h"

#include <algorithm>
#include <deque>

namespace contur {

    struct Mutex::Impl
    {
        std::optional<ProcessId> owner;
        std::size_t recursionDepth = 0;
        std::deque<ProcessId> waitQueue;
    };

    Mutex::Mutex()
        : impl_(std::make_unique<Impl>())
    {}

    Mutex::~Mutex() = default;
    Mutex::Mutex(Mutex &&) noexcept = default;
    Mutex &Mutex::operator=(Mutex &&) noexcept = default;

    Result<void> Mutex::acquire(ProcessId pid)
    {
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        if (!impl_->owner.has_value())
        {
            impl_->owner = pid;
            impl_->recursionDepth = 1;
            return Result<void>::ok();
        }

        if (impl_->owner.value() == pid)
        {
            ++impl_->recursionDepth;
            return Result<void>::ok();
        }

        if (std::find(impl_->waitQueue.begin(), impl_->waitQueue.end(), pid) == impl_->waitQueue.end())
        {
            impl_->waitQueue.push_back(pid);
        }
        return Result<void>::error(ErrorCode::ResourceBusy);
    }

    Result<void> Mutex::release(ProcessId pid)
    {
        if (!impl_->owner.has_value())
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }
        if (impl_->owner.value() != pid)
        {
            return Result<void>::error(ErrorCode::PermissionDenied);
        }

        if (impl_->recursionDepth > 1)
        {
            --impl_->recursionDepth;
            return Result<void>::ok();
        }

        impl_->owner.reset();
        impl_->recursionDepth = 0;

        if (!impl_->waitQueue.empty())
        {
            ProcessId next = impl_->waitQueue.front();
            impl_->waitQueue.pop_front();
            impl_->owner = next;
            impl_->recursionDepth = 1;
        }

        return Result<void>::ok();
    }

    Result<void> Mutex::tryAcquire(ProcessId pid)
    {
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        if (!impl_->owner.has_value())
        {
            impl_->owner = pid;
            impl_->recursionDepth = 1;
            return Result<void>::ok();
        }

        if (impl_->owner.value() == pid)
        {
            ++impl_->recursionDepth;
            return Result<void>::ok();
        }

        return Result<void>::error(ErrorCode::ResourceBusy);
    }

    std::string_view Mutex::name() const noexcept
    {
        return "Mutex";
    }

    bool Mutex::isLocked() const noexcept
    {
        return impl_->owner.has_value();
    }

    std::optional<ProcessId> Mutex::owner() const noexcept
    {
        return impl_->owner;
    }

    std::size_t Mutex::recursionDepth() const noexcept
    {
        return impl_->recursionDepth;
    }

    std::size_t Mutex::waitingCount() const noexcept
    {
        return impl_->waitQueue.size();
    }

} // namespace contur
