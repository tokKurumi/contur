/// @file device_manager.cpp
/// @brief DeviceManager implementation — I/O device registry and dispatcher.

#include "contur/io/device_manager.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace contur {

    struct DeviceManager::Impl
    {
        std::vector<std::shared_ptr<IDevice>> devices;
        std::unordered_map<DeviceId, std::shared_ptr<std::mutex>> deviceLocks;
        mutable std::mutex registryMutex;

        std::optional<std::reference_wrapper<IDevice>> findDevice(DeviceId id) noexcept
        {
            auto it = std::find_if(devices.begin(), devices.end(), [id](const auto &dev) { return dev->id() == id; });
            if (it != devices.end())
            {
                return std::ref(*it->get());
            }
            return std::nullopt;
        }

        std::optional<std::reference_wrapper<const IDevice>> findDevice(DeviceId id) const noexcept
        {
            auto it = std::find_if(devices.begin(), devices.end(), [id](const auto &dev) { return dev->id() == id; });
            if (it != devices.end())
            {
                return std::cref(*it->get());
            }
            return std::nullopt;
        }

        [[nodiscard]] std::optional<std::pair<std::shared_ptr<IDevice>, std::shared_ptr<std::mutex>>>
        lockableDevice(DeviceId id)
        {
            auto it = std::find_if(devices.begin(), devices.end(), [id](const auto &dev) { return dev->id() == id; });
            if (it == devices.end())
            {
                return std::nullopt;
            }

            auto lockIt = deviceLocks.find(id);
            if (lockIt == deviceLocks.end())
            {
                auto inserted = deviceLocks.emplace(id, std::make_shared<std::mutex>());
                lockIt = inserted.first;
            }

            return std::make_pair(*it, lockIt->second);
        }
    };

    DeviceManager::DeviceManager()
        : impl_(std::make_unique<Impl>())
    {}

    DeviceManager::~DeviceManager() = default;
    DeviceManager::DeviceManager(DeviceManager &&) noexcept = default;
    DeviceManager &DeviceManager::operator=(DeviceManager &&) noexcept = default;

    Result<void> DeviceManager::registerDevice(std::unique_ptr<IDevice> device)
    {
        if (!device)
        {
            return Result<void>::error(ErrorCode::InvalidArgument);
        }

        std::lock_guard<std::mutex> lock(impl_->registryMutex);
        if (impl_->findDevice(device->id()).has_value())
        {
            return Result<void>::error(ErrorCode::AlreadyExists);
        }

        DeviceId id = device->id();
        impl_->devices.push_back(std::shared_ptr<IDevice>(std::move(device)));
        impl_->deviceLocks.try_emplace(id, std::make_shared<std::mutex>());
        return Result<void>::ok();
    }

    Result<void> DeviceManager::unregisterDevice(DeviceId id)
    {
        std::lock_guard<std::mutex> lock(impl_->registryMutex);
        auto it = std::find_if(impl_->devices.begin(), impl_->devices.end(), [id](const auto &dev) {
            return dev->id() == id;
        });
        if (it == impl_->devices.end())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }
        impl_->devices.erase(it);
        impl_->deviceLocks.erase(id);
        return Result<void>::ok();
    }

    std::optional<std::reference_wrapper<IDevice>> DeviceManager::getDevice(DeviceId id) noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->registryMutex);
        return impl_->findDevice(id);
    }

    std::optional<std::reference_wrapper<const IDevice>> DeviceManager::getDevice(DeviceId id) const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->registryMutex);
        return impl_->findDevice(id);
    }

    Result<void> DeviceManager::write(DeviceId id, RegisterValue value)
    {
        std::optional<std::pair<std::shared_ptr<IDevice>, std::shared_ptr<std::mutex>>> deviceAndLock;
        {
            std::lock_guard<std::mutex> lock(impl_->registryMutex);
            deviceAndLock = impl_->lockableDevice(id);
        }

        if (!deviceAndLock.has_value())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        auto &[device, deviceMutex] = deviceAndLock.value();
        std::lock_guard<std::mutex> deviceLock(*deviceMutex);
        if (!device->isReady())
        {
            return Result<void>::error(ErrorCode::DeviceError);
        }
        return device->write(value);
    }

    Result<RegisterValue> DeviceManager::read(DeviceId id)
    {
        std::optional<std::pair<std::shared_ptr<IDevice>, std::shared_ptr<std::mutex>>> deviceAndLock;
        {
            std::lock_guard<std::mutex> lock(impl_->registryMutex);
            deviceAndLock = impl_->lockableDevice(id);
        }

        if (!deviceAndLock.has_value())
        {
            return Result<RegisterValue>::error(ErrorCode::NotFound);
        }

        auto &[device, deviceMutex] = deviceAndLock.value();
        std::lock_guard<std::mutex> deviceLock(*deviceMutex);
        if (!device->isReady())
        {
            return Result<RegisterValue>::error(ErrorCode::DeviceError);
        }
        return device->read();
    }

    std::size_t DeviceManager::deviceCount() const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->registryMutex);
        return impl_->devices.size();
    }

    bool DeviceManager::hasDevice(DeviceId id) const noexcept
    {
        std::lock_guard<std::mutex> lock(impl_->registryMutex);
        return impl_->findDevice(id).has_value();
    }

} // namespace contur
