/// @file test_block_allocator.cpp
/// @brief Unit tests for BlockAllocator.

#include <gtest/gtest.h>

#include "contur/fs/block_allocator.h"

using namespace contur;

TEST(BlockAllocatorTest, InitialState)
{
    BlockAllocator allocator(4);
    EXPECT_EQ(allocator.totalBlocks(), 4u);
    EXPECT_EQ(allocator.freeBlocks(), 4u);
    EXPECT_TRUE(allocator.isFree(0));
}

TEST(BlockAllocatorTest, AllocateAndFree)
{
    BlockAllocator allocator(2);

    auto b0 = allocator.allocate();
    ASSERT_TRUE(b0.isOk());
    EXPECT_EQ(b0.value(), 0u);

    auto b1 = allocator.allocate();
    ASSERT_TRUE(b1.isOk());
    EXPECT_EQ(b1.value(), 1u);

    EXPECT_EQ(allocator.freeBlocks(), 0u);

    auto free0 = allocator.free(b0.value());
    ASSERT_TRUE(free0.isOk());
    EXPECT_EQ(allocator.freeBlocks(), 1u);
    EXPECT_TRUE(allocator.isFree(0));
}

TEST(BlockAllocatorTest, AllocateOutOfMemory)
{
    BlockAllocator allocator(1);
    ASSERT_TRUE(allocator.allocate().isOk());

    auto noSpace = allocator.allocate();
    ASSERT_TRUE(noSpace.isError());
    EXPECT_EQ(noSpace.errorCode(), ErrorCode::OutOfMemory);
}

TEST(BlockAllocatorTest, FreeInvalidIndex)
{
    BlockAllocator allocator(3);

    auto invalid = allocator.free(99);
    ASSERT_TRUE(invalid.isError());
    EXPECT_EQ(invalid.errorCode(), ErrorCode::InvalidAddress);
}

TEST(BlockAllocatorTest, DoubleFreeReturnsInvalidState)
{
    BlockAllocator allocator(1);

    auto b = allocator.allocate();
    ASSERT_TRUE(b.isOk());
    ASSERT_TRUE(allocator.free(b.value()).isOk());

    auto second = allocator.free(b.value());
    ASSERT_TRUE(second.isError());
    EXPECT_EQ(second.errorCode(), ErrorCode::InvalidState);
}

TEST(BlockAllocatorTest, ResetMarksAllBlocksFree)
{
    BlockAllocator allocator(3);
    ASSERT_TRUE(allocator.allocate().isOk());
    ASSERT_TRUE(allocator.allocate().isOk());

    allocator.reset();

    EXPECT_EQ(allocator.freeBlocks(), 3u);
    EXPECT_TRUE(allocator.isFree(0));
    EXPECT_TRUE(allocator.isFree(1));
    EXPECT_TRUE(allocator.isFree(2));
}
