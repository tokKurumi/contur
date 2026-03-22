/// @file i_ipc_channel.h
/// @brief IIpcChannel interface for inter-process communication channels.

#pragma once

#include <cstddef>
#include <span>
#include <string_view>

#include "contur/core/error.h"

namespace contur {

    /// @brief Common interface for IPC channels.
    ///
    /// Implementations provide byte-level read/write semantics and
    /// lifecycle control through open/close state.
    class IIpcChannel
    {
        public:
        virtual ~IIpcChannel() = default;

        /// @brief Writes up to data.size() bytes into the channel.
        /// @param data Source bytes.
        /// @return Number of bytes written, or an error code.
        [[nodiscard]] virtual Result<std::size_t> write(std::span<const std::byte> data) = 0;

        /// @brief Reads up to buffer.size() bytes from the channel.
        /// @param buffer Destination buffer.
        /// @return Number of bytes read, or an error code.
        [[nodiscard]] virtual Result<std::size_t> read(std::span<std::byte> buffer) = 0;

        /// @brief Closes the channel and releases transient state.
        virtual void close() = 0;

        /// @brief Returns whether the channel accepts read/write operations.
        [[nodiscard]] virtual bool isOpen() const noexcept = 0;

        /// @brief Human-readable channel name.
        [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    };

} // namespace contur
