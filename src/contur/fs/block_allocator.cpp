/// @file block_allocator.cpp
/// @brief BlockAllocator implementation.

#include "contur/fs/block_allocator.h"

#include <vector>

namespace contur {

    struct BlockAllocator::Impl
    {
        std::vector<bool> freeBitmap;
        std::size_t freeCount = 0;

        explicit Impl(std::size_t totalBlocks)
            : freeBitmap(totalBlocks, true)
            , freeCount(totalBlocks)
        {}
    };

    BlockAllocator::BlockAllocator(std::size_t totalBlocks)
        : impl_(std::make_unique<Impl>(totalBlocks))
    {}

    BlockAllocator::~BlockAllocator() = default;
    BlockAllocator::BlockAllocator(BlockAllocator &&) noexcept = default;
    BlockAllocator &BlockAllocator::operator=(BlockAllocator &&) noexcept = default;

    Result<std::size_t> BlockAllocator::allocate()
    {
        if (impl_->freeCount == 0)
        {
            return Result<std::size_t>::error(ErrorCode::OutOfMemory);
        }

        for (std::size_t i = 0; i < impl_->freeBitmap.size(); ++i)
        {
            if (!impl_->freeBitmap[i])
            {
                continue;
            }

            impl_->freeBitmap[i] = false;
            --impl_->freeCount;
            return Result<std::size_t>::ok(i);
        }

        return Result<std::size_t>::error(ErrorCode::OutOfMemory);
    }

    Result<void> BlockAllocator::free(std::size_t blockIndex)
    {
        if (blockIndex >= impl_->freeBitmap.size())
        {
            return Result<void>::error(ErrorCode::InvalidAddress);
        }

        if (impl_->freeBitmap[blockIndex])
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        impl_->freeBitmap[blockIndex] = true;
        ++impl_->freeCount;
        return Result<void>::ok();
    }

    bool BlockAllocator::isFree(std::size_t blockIndex) const noexcept
    {
        if (blockIndex >= impl_->freeBitmap.size())
        {
            return false;
        }

        return impl_->freeBitmap[blockIndex];
    }

    std::size_t BlockAllocator::totalBlocks() const noexcept
    {
        return impl_->freeBitmap.size();
    }

    std::size_t BlockAllocator::freeBlocks() const noexcept
    {
        return impl_->freeCount;
    }

    void BlockAllocator::reset()
    {
        for (std::size_t i = 0; i < impl_->freeBitmap.size(); ++i)
        {
            impl_->freeBitmap[i] = true;
        }
        impl_->freeCount = impl_->freeBitmap.size();
    }

} // namespace contur
