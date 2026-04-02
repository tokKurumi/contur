/// @file virtual_memory.h
/// @brief VirtualMemory — manages virtual address slots for processes.

#pragma once

#include <memory>

#include "contur/memory/i_virtual_memory.h"

namespace contur {

    // Forward declarations
    class IMMU;

    /// @brief Concrete virtual memory manager.
    ///
    /// Manages slot allocation and delegates actual memory operations
    /// to the underlying IMMU. Each process gets a dedicated virtual
    /// address range (slot) for its code and data.
    class VirtualMemory final : public IVirtualMemory
    {
        public:
        /// @brief Constructs virtual memory backed by the given MMU.
        /// @param mmu The MMU to use for address translation (non-owning reference).
        /// @param maxSlots Maximum number of concurrent process slots.
        explicit VirtualMemory(IMMU &mmu, std::size_t maxSlots);

        /// @brief Destroys virtual memory manager.
        ~VirtualMemory() override;

        // Non-copyable, movable
        VirtualMemory(const VirtualMemory &) = delete;
        VirtualMemory &operator=(const VirtualMemory &) = delete;
        /// @brief Move-constructs virtual memory manager state.
        VirtualMemory(VirtualMemory &&) noexcept;

        /// @brief Move-assigns virtual memory manager state.
        VirtualMemory &operator=(VirtualMemory &&) noexcept;

        // IVirtualMemory interface
        /// @copydoc IVirtualMemory::allocateSlot
        [[nodiscard]] Result<MemoryAddress> allocateSlot(ProcessId processId, std::size_t size) override;

        /// @copydoc IVirtualMemory::freeSlot
        [[nodiscard]] Result<void> freeSlot(ProcessId processId) override;

        /// @copydoc IVirtualMemory::loadSegment
        [[nodiscard]] Result<void> loadSegment(ProcessId processId, const std::vector<Block> &data) override;

        /// @copydoc IVirtualMemory::readSegment
        [[nodiscard]] Result<std::vector<Block>> readSegment(ProcessId processId) const override;

        /// @copydoc IVirtualMemory::totalSlots
        [[nodiscard]] std::size_t totalSlots() const noexcept override;

        /// @copydoc IVirtualMemory::freeSlots
        [[nodiscard]] std::size_t freeSlots() const noexcept override;

        /// @copydoc IVirtualMemory::hasSlot
        [[nodiscard]] bool hasSlot(ProcessId processId) const noexcept override;

        /// @copydoc IVirtualMemory::slotSize
        [[nodiscard]] std::size_t slotSize(ProcessId processId) const noexcept override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
