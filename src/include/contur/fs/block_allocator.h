/// @file block_allocator.h
/// @brief Bitmap-style block allocator for the simulated filesystem disk.

#pragma once

#include <memory>

#include "contur/core/error.h"

namespace contur {

    /// @brief Allocates and frees fixed-size disk blocks.
    ///
    /// Tracks free/used state for each block and provides O(n) first-fit
    /// allocation suitable for educational simulation.
    class BlockAllocator final
    {
        public:
        /// @brief Creates a block allocator over a fixed number of blocks.
        /// @param totalBlocks Total number of allocatable blocks.
        explicit BlockAllocator(std::size_t totalBlocks);

        /// @brief Destroys block allocator.
        ~BlockAllocator();

        /// @brief Copy construction is disabled.
        BlockAllocator(const BlockAllocator &) = delete;

        /// @brief Copy assignment is disabled.
        BlockAllocator &operator=(const BlockAllocator &) = delete;
        /// @brief Move-constructs allocator state.
        BlockAllocator(BlockAllocator &&) noexcept;

        /// @brief Move-assigns allocator state.
        BlockAllocator &operator=(BlockAllocator &&) noexcept;

        /// @brief Allocates one free block.
        /// @return Block index on success; OutOfMemory if no free blocks remain.
        [[nodiscard]] Result<std::size_t> allocate();

        /// @brief Frees a previously allocated block.
        /// @param blockIndex Index of block to free.
        /// @return Ok on success; InvalidAddress or InvalidState on misuse.
        [[nodiscard]] Result<void> free(std::size_t blockIndex);

        /// @brief Returns whether block is free.
        /// @param blockIndex Block index to query.
        /// @return True when free; false for used or out-of-range indices.
        [[nodiscard]] bool isFree(std::size_t blockIndex) const noexcept;

        /// @brief Total number of blocks managed by allocator.
        /// @return Total block capacity.
        [[nodiscard]] std::size_t totalBlocks() const noexcept;

        /// @brief Number of currently free blocks.
        /// @return Free block count.
        [[nodiscard]] std::size_t freeBlocks() const noexcept;

        /// @brief Resets allocator state, marking every block as free.
        void reset();

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
