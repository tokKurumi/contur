/// @file ipc_manager.cpp
/// @brief IpcManager implementation.

#include "contur/ipc/ipc_manager.h"

#include <mutex>
#include <unordered_map>
#include <utility>

#include "contur/ipc/message_queue.h"
#include "contur/ipc/pipe.h"
#include "contur/ipc/shared_memory.h"

namespace contur {

    struct IpcManager::Impl
    {
        std::unordered_map<std::string, std::unique_ptr<IIpcChannel>> channels;
        mutable std::mutex mutex;
    };

    IpcManager::IpcManager()
        : impl_(std::make_unique<Impl>())
    {}

    IpcManager::~IpcManager() = default;
    IpcManager::IpcManager(IpcManager &&) noexcept = default;
    IpcManager &IpcManager::operator=(IpcManager &&) noexcept = default;

    Result<void> IpcManager::createPipe(const std::string &name, std::size_t capacity)
    {
        if (name.empty())
        {
            return Result<void>::error(ErrorCode::InvalidArgument);
        }

        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (impl_->channels.find(name) != impl_->channels.end())
        {
            return Result<void>::ok();
        }

        impl_->channels.emplace(name, std::make_unique<Pipe>(name, capacity));
        return Result<void>::ok();
    }

    Result<void> IpcManager::createSharedMemory(const std::string &name, std::size_t bytes)
    {
        if (name.empty())
        {
            return Result<void>::error(ErrorCode::InvalidArgument);
        }

        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (impl_->channels.find(name) != impl_->channels.end())
        {
            return Result<void>::ok();
        }

        impl_->channels.emplace(name, std::make_unique<SharedMemory>(name, bytes));
        return Result<void>::ok();
    }

    Result<void> IpcManager::createMessageQueue(const std::string &name, std::size_t maxMessages, bool priorityMode)
    {
        if (name.empty())
        {
            return Result<void>::error(ErrorCode::InvalidArgument);
        }

        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (impl_->channels.find(name) != impl_->channels.end())
        {
            return Result<void>::ok();
        }

        impl_->channels.emplace(name, std::make_unique<MessageQueue>(name, maxMessages, priorityMode));
        return Result<void>::ok();
    }

    Result<std::reference_wrapper<IIpcChannel>> IpcManager::getChannel(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        auto it = impl_->channels.find(name);
        if (it == impl_->channels.end())
        {
            return Result<std::reference_wrapper<IIpcChannel>>::error(ErrorCode::NotFound);
        }

        return Result<std::reference_wrapper<IIpcChannel>>::ok(std::ref(*it->second));
    }

    Result<void> IpcManager::destroyChannel(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        auto erased = impl_->channels.erase(name);
        if (erased == 0)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        return Result<void>::ok();
    }

    bool IpcManager::exists(const std::string &name) const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->channels.find(name) != impl_->channels.end();
    }

    std::size_t IpcManager::channelCount() const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->channels.size();
    }

} // namespace contur
