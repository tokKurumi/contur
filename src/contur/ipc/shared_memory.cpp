/// @file shared_memory.cpp
/// @brief SharedMemory IPC channel implementation.

#include "contur/ipc/shared_memory.h"

#include <algorithm>
#include <mutex>
#include <unordered_set>
#include <utility>
#include <vector>

namespace contur {

    struct SharedMemory::Impl
    {
        std::string name;
        bool open = true;
        std::vector<std::byte> region;
        std::unordered_set<ProcessId> attached;
        mutable std::mutex mutex;
    };

    SharedMemory::SharedMemory(std::string name, std::size_t bytes)
        : impl_(std::make_unique<Impl>())
    {
        impl_->name = std::move(name);
        impl_->region.resize(std::max<std::size_t>(1, bytes));
    }

    SharedMemory::~SharedMemory() = default;
    SharedMemory::SharedMemory(SharedMemory &&) noexcept = default;
    SharedMemory &SharedMemory::operator=(SharedMemory &&) noexcept = default;

    Result<std::size_t> SharedMemory::write(std::span<const std::byte> data)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (!impl_->open)
        {
            return Result<std::size_t>::error(ErrorCode::InvalidState);
        }
        if (data.empty())
        {
            return Result<std::size_t>::ok(0);
        }

        const std::size_t toWrite = std::min(data.size(), impl_->region.size());
        std::copy_n(data.begin(), toWrite, impl_->region.begin());
        return Result<std::size_t>::ok(toWrite);
    }

    Result<std::size_t> SharedMemory::read(std::span<std::byte> buffer)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (!impl_->open)
        {
            return Result<std::size_t>::error(ErrorCode::InvalidState);
        }
        if (buffer.empty())
        {
            return Result<std::size_t>::ok(0);
        }

        const std::size_t toRead = std::min(buffer.size(), impl_->region.size());
        std::copy_n(impl_->region.begin(), toRead, buffer.begin());
        return Result<std::size_t>::ok(toRead);
    }

    void SharedMemory::close()
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        impl_->open = false;
        std::fill(impl_->region.begin(), impl_->region.end(), std::byte{0});
        impl_->attached.clear();
    }

    bool SharedMemory::isOpen() const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->open;
    }

    std::string_view SharedMemory::name() const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->name;
    }

    Result<void> SharedMemory::attach(ProcessId pid)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (!impl_->open)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        impl_->attached.insert(pid);
        return Result<void>::ok();
    }

    Result<void> SharedMemory::detach(ProcessId pid)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (pid == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        auto erased = impl_->attached.erase(pid);
        if (erased == 0)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        return Result<void>::ok();
    }

    bool SharedMemory::isAttached(ProcessId pid) const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->attached.find(pid) != impl_->attached.end();
    }

    std::size_t SharedMemory::attachedCount() const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->attached.size();
    }

    std::size_t SharedMemory::size() const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->region.size();
    }

} // namespace contur
