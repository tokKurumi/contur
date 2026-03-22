/// @file test_ipc_manager.cpp
/// @brief Unit tests for IpcManager.

#include <vector>

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/ipc/ipc_manager.h"

using namespace contur;

TEST(IpcManagerTest, CreatePipeAndLookup)
{
    IpcManager manager;

    ASSERT_TRUE(manager.createPipe("pipe-a", 8).isOk());
    auto channel = manager.getChannel("pipe-a");

    ASSERT_TRUE(channel.isOk());
    EXPECT_EQ(channel.value().get().name(), "pipe-a");
    EXPECT_EQ(manager.channelCount(), 1u);
    EXPECT_TRUE(manager.exists("pipe-a"));
}

TEST(IpcManagerTest, CreateSharedMemoryAndMessageQueue)
{
    IpcManager manager;

    ASSERT_TRUE(manager.createSharedMemory("shm-a", 64).isOk());
    ASSERT_TRUE(manager.createMessageQueue("mq-a", 4, true).isOk());

    EXPECT_TRUE(manager.exists("shm-a"));
    EXPECT_TRUE(manager.exists("mq-a"));
    EXPECT_EQ(manager.channelCount(), 2u);
}

TEST(IpcManagerTest, DuplicateCreateReturnsOkWithoutGrowingRegistry)
{
    IpcManager manager;

    ASSERT_TRUE(manager.createPipe("dup", 8).isOk());
    ASSERT_TRUE(manager.createPipe("dup", 8).isOk());

    EXPECT_EQ(manager.channelCount(), 1u);
}

TEST(IpcManagerTest, EmptyNameRejected)
{
    IpcManager manager;

    auto pipe = manager.createPipe("", 8);
    auto shm = manager.createSharedMemory("", 8);
    auto mq = manager.createMessageQueue("", 8, false);

    EXPECT_TRUE(pipe.isError());
    EXPECT_EQ(pipe.errorCode(), ErrorCode::InvalidArgument);
    EXPECT_TRUE(shm.isError());
    EXPECT_EQ(shm.errorCode(), ErrorCode::InvalidArgument);
    EXPECT_TRUE(mq.isError());
    EXPECT_EQ(mq.errorCode(), ErrorCode::InvalidArgument);
}

TEST(IpcManagerTest, GetChannelReturnsNotFoundForUnknownName)
{
    IpcManager manager;

    auto result = manager.getChannel("missing");

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::NotFound);
}

TEST(IpcManagerTest, DestroyChannelRemovesEntry)
{
    IpcManager manager;
    ASSERT_TRUE(manager.createPipe("pipe-b", 8).isOk());

    auto destroyed = manager.destroyChannel("pipe-b");

    EXPECT_TRUE(destroyed.isOk());
    EXPECT_FALSE(manager.exists("pipe-b"));
    EXPECT_EQ(manager.channelCount(), 0u);
}

TEST(IpcManagerTest, DestroyUnknownReturnsNotFound)
{
    IpcManager manager;

    auto result = manager.destroyChannel("missing");

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::NotFound);
}
