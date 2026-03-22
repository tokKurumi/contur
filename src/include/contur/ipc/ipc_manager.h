/// @file ipc_manager.h
/// @brief IpcManager registry for IPC channels.

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "contur/ipc/i_ipc_channel.h"

namespace contur {

    /// @brief Registry/mediator for named IPC channels.
    ///
    /// IpcManager owns channel lifetimes and provides lookup by name.
    class IpcManager
    {
        public:
        IpcManager();
        ~IpcManager();

        IpcManager(const IpcManager &) = delete;
        IpcManager &operator=(const IpcManager &) = delete;
        IpcManager(IpcManager &&) noexcept;
        IpcManager &operator=(IpcManager &&) noexcept;

        /// @brief Creates a pipe channel if it does not already exist.
        /// @param name Channel name.
        /// @param capacity Pipe capacity in bytes.
        /// @return Ok on success, InvalidArgument on invalid input.
        [[nodiscard]] Result<void> createPipe(const std::string &name, std::size_t capacity = 1024);

        /// @brief Creates a shared-memory channel if it does not already exist.
        /// @param name Channel name.
        /// @param bytes Shared-memory size in bytes.
        /// @return Ok on success, InvalidArgument on invalid input.
        [[nodiscard]] Result<void> createSharedMemory(const std::string &name, std::size_t bytes);

        /// @brief Creates a message queue channel if it does not already exist.
        /// @param name Channel name.
        /// @param maxMessages Maximum queued messages.
        /// @param priorityMode True enables priority ordering.
        /// @return Ok on success, InvalidArgument on invalid input.
        [[nodiscard]] Result<void>
        createMessageQueue(const std::string &name, std::size_t maxMessages = 64, bool priorityMode = false);

        /// @brief Looks up a channel by name.
        /// @return Reference to channel or NotFound.
        [[nodiscard]] Result<std::reference_wrapper<IIpcChannel>> getChannel(const std::string &name);

        /// @brief Destroys a channel by name.
        /// @return Ok on success or NotFound.
        [[nodiscard]] Result<void> destroyChannel(const std::string &name);

        /// @brief Checks whether a named channel exists.
        [[nodiscard]] bool exists(const std::string &name) const noexcept;

        /// @brief Number of registered channels.
        [[nodiscard]] std::size_t channelCount() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
