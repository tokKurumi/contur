/// @file critical_section.cpp
/// @brief CriticalSection implementation.

#include "contur/sync/critical_section.h"

#include "contur/sync/mutex.h"

namespace contur {

    struct CriticalSection::Impl
    {
        std::unique_ptr<ISyncPrimitive> primitive;
    };

    CriticalSection::CriticalSection(std::unique_ptr<ISyncPrimitive> primitive)
        : impl_(std::make_unique<Impl>())
    {
        if (primitive)
        {
            impl_->primitive = std::move(primitive);
        }
        else
        {
            impl_->primitive = std::make_unique<Mutex>();
        }
    }

    CriticalSection::~CriticalSection() = default;
    CriticalSection::CriticalSection(CriticalSection &&) noexcept = default;
    CriticalSection &CriticalSection::operator=(CriticalSection &&) noexcept = default;

    Result<void> CriticalSection::acquire(ProcessId pid)
    {
        return impl_->primitive->acquire(pid);
    }

    Result<void> CriticalSection::release(ProcessId pid)
    {
        return impl_->primitive->release(pid);
    }

    Result<void> CriticalSection::tryAcquire(ProcessId pid)
    {
        return impl_->primitive->tryAcquire(pid);
    }

    std::string_view CriticalSection::name() const noexcept
    {
        return "CriticalSection";
    }

    Result<void> CriticalSection::enter(ProcessId pid)
    {
        return acquire(pid);
    }

    Result<void> CriticalSection::leave(ProcessId pid)
    {
        return release(pid);
    }

    Result<void> CriticalSection::tryEnter(ProcessId pid)
    {
        return tryAcquire(pid);
    }

} // namespace contur
