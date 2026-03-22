/// @file pipe.h
/// @brief Pipe IPC channel implementation.

#pragma once

#include <memory>
#include <string>

#include "contur/ipc/i_ipc_channel.h"

namespace contur {

    /// @brief Unidirectional byte-stream IPC channel with bounded capacity.
    ///
    /// Pipe stores bytes in FIFO order. Reads consume bytes from the front.
    class Pipe final : public IIpcChannel
    {
        public:
        /// @brief Creates a pipe with a logical name and capacity.
        /// @param name Channel name used by registries and diagnostics.
        /// @param capacity Maximum number of buffered bytes.
        explicit Pipe(std::string name, std::size_t capacity = 1024);
        ~Pipe() override;

        Pipe(const Pipe &) = delete;
        Pipe &operator=(const Pipe &) = delete;
        Pipe(Pipe &&) noexcept;
        Pipe &operator=(Pipe &&) noexcept;

        /// @brief Writes bytes into the pipe buffer.
        /// @return Written byte count, BufferFull if no free space,
        /// InvalidState if the pipe is closed.
        [[nodiscard]] Result<std::size_t> write(std::span<const std::byte> data) override;

        /// @brief Reads bytes from the pipe buffer in FIFO order.
        /// @return Read byte count, BufferEmpty if no data,
        /// InvalidState if the pipe is closed.
        [[nodiscard]] Result<std::size_t> read(std::span<std::byte> buffer) override;

        /// @brief Closes the pipe and clears buffered data.
        void close() override;

        /// @brief Returns whether the pipe is open.
        [[nodiscard]] bool isOpen() const noexcept override;

        /// @brief Pipe name.
        [[nodiscard]] std::string_view name() const noexcept override;

        /// @brief Maximum number of bytes that can be buffered.
        [[nodiscard]] std::size_t capacity() const noexcept;

        /// @brief Current number of buffered bytes.
        [[nodiscard]] std::size_t size() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
