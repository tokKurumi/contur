/// @file test_pipe.cpp
/// @brief Unit tests for Pipe IPC channel.

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/ipc/pipe.h"

using namespace contur;

TEST(PipeTest, ConstructorSetsNameCapacityAndOpenState)
{
    Pipe pipe("p0", 8);

    EXPECT_EQ(pipe.name(), "p0");
    EXPECT_EQ(pipe.capacity(), 8u);
    EXPECT_TRUE(pipe.isOpen());
    EXPECT_EQ(pipe.size(), 0u);
}

TEST(PipeTest, WriteAndReadPreserveOrder)
{
    Pipe pipe("p1", 8);
    std::vector<std::byte> in = {std::byte{1}, std::byte{2}, std::byte{3}};

    auto written = pipe.write(in);
    ASSERT_TRUE(written.isOk());
    EXPECT_EQ(written.value(), 3u);

    std::vector<std::byte> out(3);
    auto read = pipe.read(out);
    ASSERT_TRUE(read.isOk());
    EXPECT_EQ(read.value(), 3u);
    EXPECT_EQ(out[0], std::byte{1});
    EXPECT_EQ(out[1], std::byte{2});
    EXPECT_EQ(out[2], std::byte{3});
    EXPECT_EQ(pipe.size(), 0u);
}

TEST(PipeTest, WriteReturnsBufferFullWhenNoSpace)
{
    Pipe pipe("p2", 2);
    const std::vector<std::byte> full = {std::byte{10}, std::byte{11}};
    const std::vector<std::byte> one = {std::byte{12}};

    ASSERT_TRUE(pipe.write(full).isOk());
    auto result = pipe.write(one);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::BufferFull);
}

TEST(PipeTest, ReadReturnsBufferEmptyWhenNoData)
{
    Pipe pipe("p3", 4);
    std::vector<std::byte> out(1);

    auto result = pipe.read(out);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::BufferEmpty);
}

TEST(PipeTest, WriteMayBePartialWhenInputExceedsAvailableSpace)
{
    Pipe pipe("p4", 3);
    const std::vector<std::byte> data = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};

    auto result = pipe.write(data);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 3u);
    EXPECT_EQ(pipe.size(), 3u);
}

TEST(PipeTest, CloseDisablesReadAndWrite)
{
    Pipe pipe("p5", 4);
    const std::vector<std::byte> one = {std::byte{1}};
    pipe.close();

    auto writeResult = pipe.write(one);
    std::vector<std::byte> out(1);
    auto readResult = pipe.read(out);

    EXPECT_FALSE(pipe.isOpen());
    EXPECT_TRUE(writeResult.isError());
    EXPECT_EQ(writeResult.errorCode(), ErrorCode::InvalidState);
    EXPECT_TRUE(readResult.isError());
    EXPECT_EQ(readResult.errorCode(), ErrorCode::InvalidState);
}
