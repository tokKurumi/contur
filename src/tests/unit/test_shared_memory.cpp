/// @file test_shared_memory.cpp
/// @brief Unit tests for SharedMemory IPC channel.

#include <vector>

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/ipc/shared_memory.h"

using namespace contur;

TEST(SharedMemoryTest, ConstructorInitializesRegion)
{
    SharedMemory shm("shm0", 16);

    EXPECT_EQ(shm.name(), "shm0");
    EXPECT_TRUE(shm.isOpen());
    EXPECT_EQ(shm.size(), 16u);
    EXPECT_EQ(shm.attachedCount(), 0u);
}

TEST(SharedMemoryTest, WriteAndReadRoundTrip)
{
    SharedMemory shm("shm1", 4);
    std::vector<std::byte> in = {std::byte{10}, std::byte{20}, std::byte{30}, std::byte{40}};

    auto writeResult = shm.write(in);
    ASSERT_TRUE(writeResult.isOk());
    EXPECT_EQ(writeResult.value(), 4u);

    std::vector<std::byte> out(4);
    auto readResult = shm.read(out);
    ASSERT_TRUE(readResult.isOk());
    EXPECT_EQ(readResult.value(), 4u);
    EXPECT_EQ(out, in);
}

TEST(SharedMemoryTest, WriteCanBePartialWhenInputLargerThanRegion)
{
    SharedMemory shm("shm2", 2);
    std::vector<std::byte> in = {std::byte{1}, std::byte{2}, std::byte{3}};

    auto result = shm.write(in);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 2u);
}

TEST(SharedMemoryTest, AttachDetachAndMembership)
{
    SharedMemory shm("shm3", 8);

    ASSERT_TRUE(shm.attach(1).isOk());
    ASSERT_TRUE(shm.attach(2).isOk());
    EXPECT_TRUE(shm.isAttached(1));
    EXPECT_TRUE(shm.isAttached(2));
    EXPECT_EQ(shm.attachedCount(), 2u);

    ASSERT_TRUE(shm.detach(1).isOk());
    EXPECT_FALSE(shm.isAttached(1));
    EXPECT_EQ(shm.attachedCount(), 1u);
}

TEST(SharedMemoryTest, DetachUnknownProcessReturnsNotFound)
{
    SharedMemory shm("shm4", 8);

    auto result = shm.detach(123);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::NotFound);
}

TEST(SharedMemoryTest, InvalidPidRejectedOnAttachDetach)
{
    SharedMemory shm("shm5", 8);

    auto attach = shm.attach(INVALID_PID);
    auto detach = shm.detach(INVALID_PID);

    EXPECT_TRUE(attach.isError());
    EXPECT_EQ(attach.errorCode(), ErrorCode::InvalidPid);
    EXPECT_TRUE(detach.isError());
    EXPECT_EQ(detach.errorCode(), ErrorCode::InvalidPid);
}

TEST(SharedMemoryTest, CloseDisablesOperations)
{
    SharedMemory shm("shm6", 8);
    const std::vector<std::byte> one = {std::byte{1}};
    shm.close();

    auto writeResult = shm.write(one);
    std::vector<std::byte> out(1);
    auto readResult = shm.read(out);
    auto attachResult = shm.attach(1);

    EXPECT_FALSE(shm.isOpen());
    EXPECT_TRUE(writeResult.isError());
    EXPECT_EQ(writeResult.errorCode(), ErrorCode::InvalidState);
    EXPECT_TRUE(readResult.isError());
    EXPECT_EQ(readResult.errorCode(), ErrorCode::InvalidState);
    EXPECT_TRUE(attachResult.isError());
    EXPECT_EQ(attachResult.errorCode(), ErrorCode::InvalidState);
}
