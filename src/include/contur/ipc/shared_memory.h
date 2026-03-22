/// @file shared_memory.h
/// @brief SharedMemory IPC channel implementation.

#pragma once

#include <memory>
#include <string>

#include "contur/core/types.h"

#include "contur/ipc/i_ipc_channel.h"

namespace contur {

    /// @brief Named shared-memory IPC channel.
    ///
    /// SharedMemory exposes a fixed-size byte region that can be attached
    /// by multiple processes. Attach/detach tracking is managed internally.
    class SharedMemory final : public IIpcChannel
    {
        public:
        /// @brief Creates a named shared-memory region.
        /// @param name Channel name.
        /// @param bytes Size of memory region in bytes.
        explicit SharedMemory(std::string name, std::size_t bytes);
        ~SharedMemory() override;

        SharedMemory(const SharedMemory &) = delete;
        SharedMemory &operator=(const SharedMemory &) = delete;
        SharedMemory(SharedMemory &&) noexcept;
        SharedMemory &operator=(SharedMemory &&) noexcept;

        /// @brief Writes bytes into the beginning of the shared region.
        /// @return Number of bytes written (can be partial), or error.
        [[nodiscard]] Result<std::size_t> write(std::span<const std::byte> data) override;

        /// @brief Reads bytes from the beginning of the shared region.
        /// @return Number of bytes read (can be partial), or error.
        [[nodiscard]] Result<std::size_t> read(std::span<std::byte> buffer) override;

        /// @brief Closes the region, clears attachments, and resets data.
        void close() override;

        /// @brief Returns whether the region is open.
        [[nodiscard]] bool isOpen() const noexcept override;

        /// @brief Region name.
        [[nodiscard]] std::string_view name() const noexcept override;

        /// @brief Attaches a process to this region.
        /// @param pid Process ID.
        /// @return Ok on success, or InvalidPid/InvalidState.
        [[nodiscard]] Result<void> attach(ProcessId pid);

        /// @brief Detaches a process from this region.
        /// @param pid Process ID.
        /// @return Ok on success, or InvalidPid/NotFound.
        [[nodiscard]] Result<void> detach(ProcessId pid);

        /// @brief Returns true if process is attached.
        [[nodiscard]] bool isAttached(ProcessId pid) const noexcept;

        /// @brief Number of attached processes.
        [[nodiscard]] std::size_t attachedCount() const noexcept;

        /// @brief Region size in bytes.
        [[nodiscard]] std::size_t size() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
