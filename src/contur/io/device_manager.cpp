/// @file device_manager.cpp
/// @brief DeviceManager implementation — I/O device registry and dispatcher.

#include "contur/io/device_manager.h"

#include <algorithm>
#include <functional>
#include <optional>

namespace contur {

    struct DeviceManager::Impl
    {
        std::vector<std::unique_ptr<IDevice>> devices;

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
        if (impl_->findDevice(device->id()).has_value())
        {
            return Result<void>::error(ErrorCode::AlreadyExists);
        }
        impl_->devices.push_back(std::move(device));
        return Result<void>::ok();
    }

    Result<void> DeviceManager::unregisterDevice(DeviceId id)
    {
        auto it = std::find_if(impl_->devices.begin(), impl_->devices.end(), [id](const auto &dev) {
            return dev->id() == id;
        });
        if (it == impl_->devices.end())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }
        impl_->devices.erase(it);
        return Result<void>::ok();
    }

    std::optional<std::reference_wrapper<IDevice>> DeviceManager::getDevice(DeviceId id) noexcept
    {
        return impl_->findDevice(id);
    }

    std::optional<std::reference_wrapper<const IDevice>> DeviceManager::getDevice(DeviceId id) const noexcept
    {
        return impl_->findDevice(id);
    }

    Result<void> DeviceManager::write(DeviceId id, RegisterValue value)
    {
        auto device = impl_->findDevice(id);
        if (!device.has_value())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }
        if (!device->get().isReady())
        {
            return Result<void>::error(ErrorCode::DeviceError);
        }
        return device->get().write(value);
    }

    Result<RegisterValue> DeviceManager::read(DeviceId id)
    {
        auto device = impl_->findDevice(id);
        if (!device.has_value())
        {
            return Result<RegisterValue>::error(ErrorCode::NotFound);
        }
        if (!device->get().isReady())
        {
            return Result<RegisterValue>::error(ErrorCode::DeviceError);
        }
        return device->get().read();
    }

    std::size_t DeviceManager::deviceCount() const noexcept
    {
        return impl_->devices.size();
    }

    bool DeviceManager::hasDevice(DeviceId id) const noexcept
    {
        return impl_->findDevice(id).has_value();
    }

} // namespace contur
