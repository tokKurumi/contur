/// @file message_queue.h
/// @brief MessageQueue IPC channel implementation.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "contur/ipc/i_ipc_channel.h"

namespace contur {

    /// @brief Typed message used by MessageQueue.
    struct Message
    {
        /// @brief User-defined message type.
        std::uint32_t type = 0;
        /// @brief Scheduling priority (higher value = higher priority).
        std::uint32_t priority = 0;
        /// @brief Opaque message payload bytes.
        std::vector<std::byte> payload;
    };

    /// @brief FIFO/priority message queue IPC channel.
    ///
    /// The queue can operate in FIFO mode or in priority mode, where
    /// higher-priority messages are received first.
    class MessageQueue final : public IIpcChannel
    {
        public:
        /// @brief Creates a message queue.
        /// @param name Queue name.
        /// @param maxMessages Maximum number of queued messages.
        /// @param priorityMode True enables priority ordering.
        explicit MessageQueue(std::string name, std::size_t maxMessages = 64, bool priorityMode = false);

        /// @brief Destroys message queue.
        ~MessageQueue() override;

        /// @brief Copy construction is disabled.
        MessageQueue(const MessageQueue &) = delete;

        /// @brief Copy assignment is disabled.
        MessageQueue &operator=(const MessageQueue &) = delete;
        /// @brief Move-constructs queue state.
        MessageQueue(MessageQueue &&) noexcept;

        /// @brief Move-assigns queue state.
        MessageQueue &operator=(MessageQueue &&) noexcept;

        /// @brief Enqueues a raw payload as a message.
        /// @return Payload byte count on success, or error.
        [[nodiscard]] Result<std::size_t> write(std::span<const std::byte> data) override;

        /// @brief Dequeues one message and copies its payload to buffer.
        /// @return Copied byte count on success, or error.
        [[nodiscard]] Result<std::size_t> read(std::span<std::byte> buffer) override;

        /// @brief Closes queue and clears pending messages.
        void close() override;

        /// @brief Returns whether queue is open.
        [[nodiscard]] bool isOpen() const noexcept override;

        /// @brief Queue name.
        [[nodiscard]] std::string_view name() const noexcept override;

        /// @brief Enqueues a typed message.
        /// @return Ok on success, or BufferFull/InvalidState.
        [[nodiscard]] Result<void> send(const Message &message);

        /// @brief Dequeues one message.
        /// @return Message on success, or BufferEmpty/InvalidState.
        [[nodiscard]] Result<Message> receive();

        /// @brief Number of currently queued messages.
        [[nodiscard]] std::size_t size() const noexcept;

        /// @brief Maximum queue length in messages.
        [[nodiscard]] std::size_t maxMessages() const noexcept;

        /// @brief True when priority ordering is enabled.
        [[nodiscard]] bool isPriorityMode() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
