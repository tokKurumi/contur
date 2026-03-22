/// @file test_message_queue.cpp
/// @brief Unit tests for MessageQueue IPC channel.

#include <vector>

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/ipc/message_queue.h"

using namespace contur;

TEST(MessageQueueTest, ConstructorSetsProperties)
{
    MessageQueue queue("mq0", 3, true);

    EXPECT_EQ(queue.name(), "mq0");
    EXPECT_TRUE(queue.isOpen());
    EXPECT_EQ(queue.maxMessages(), 3u);
    EXPECT_TRUE(queue.isPriorityMode());
    EXPECT_EQ(queue.size(), 0u);
}

TEST(MessageQueueTest, SendReceiveFifoOrder)
{
    MessageQueue queue("mq1", 4, false);

    ASSERT_TRUE(queue.send({1, 0, {std::byte{10}}}).isOk());
    ASSERT_TRUE(queue.send({2, 0, {std::byte{20}}}).isOk());

    auto m1 = queue.receive();
    auto m2 = queue.receive();

    ASSERT_TRUE(m1.isOk());
    ASSERT_TRUE(m2.isOk());
    EXPECT_EQ(m1.value().type, 1u);
    EXPECT_EQ(m1.value().payload[0], std::byte{10});
    EXPECT_EQ(m2.value().type, 2u);
    EXPECT_EQ(m2.value().payload[0], std::byte{20});
}

TEST(MessageQueueTest, SendReceivePriorityOrder)
{
    MessageQueue queue("mq2", 4, true);

    ASSERT_TRUE(queue.send({1, 1, {std::byte{1}}}).isOk());
    ASSERT_TRUE(queue.send({2, 5, {std::byte{2}}}).isOk());
    ASSERT_TRUE(queue.send({3, 3, {std::byte{3}}}).isOk());

    auto a = queue.receive();
    auto b = queue.receive();
    auto c = queue.receive();

    ASSERT_TRUE(a.isOk());
    ASSERT_TRUE(b.isOk());
    ASSERT_TRUE(c.isOk());
    EXPECT_EQ(a.value().type, 2u);
    EXPECT_EQ(b.value().type, 3u);
    EXPECT_EQ(c.value().type, 1u);
}

TEST(MessageQueueTest, SendReturnsBufferFullAtCapacity)
{
    MessageQueue queue("mq3", 1, false);

    ASSERT_TRUE(queue.send({1, 0, {}}).isOk());
    auto full = queue.send({2, 0, {}});

    EXPECT_TRUE(full.isError());
    EXPECT_EQ(full.errorCode(), ErrorCode::BufferFull);
}

TEST(MessageQueueTest, ReceiveReturnsBufferEmptyWhenNoMessages)
{
    MessageQueue queue("mq4", 2, false);

    auto result = queue.receive();

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::BufferEmpty);
}

TEST(MessageQueueTest, ByteLevelWriteReadWorks)
{
    MessageQueue queue("mq5", 2, false);
    std::vector<std::byte> in = {std::byte{7}, std::byte{8}};

    auto write = queue.write(in);
    ASSERT_TRUE(write.isOk());
    EXPECT_EQ(write.value(), 2u);

    std::vector<std::byte> out(2);
    auto read = queue.read(out);
    ASSERT_TRUE(read.isOk());
    EXPECT_EQ(read.value(), 2u);
    EXPECT_EQ(out, in);
}

TEST(MessageQueueTest, CloseDisablesOperations)
{
    MessageQueue queue("mq6", 2, false);
    queue.close();

    auto send = queue.send({1, 0, {}});
    auto receive = queue.receive();

    EXPECT_FALSE(queue.isOpen());
    EXPECT_TRUE(send.isError());
    EXPECT_EQ(send.errorCode(), ErrorCode::InvalidState);
    EXPECT_TRUE(receive.isError());
    EXPECT_EQ(receive.errorCode(), ErrorCode::InvalidState);
}
