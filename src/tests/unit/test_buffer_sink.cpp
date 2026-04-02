/// @file test_buffer_sink.cpp
/// @brief Unit tests for BufferSink.

#include <vector>

#include <gtest/gtest.h>

#include "contur/tracing/buffer_sink.h"

using namespace contur;

TEST(BufferSinkTest, WriteSnapshotAndClear)
{
    BufferSink sink;

    sink.write(makeTraceEvent(1, "Kernel", "createProcess", "pid=1", 0));
    sink.write(makeTraceEvent(2, "Dispatcher", "dispatch", "lane=0", 1));

    EXPECT_EQ(sink.size(), 2u);

    auto events = sink.snapshot();
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].timestamp, 1u);
    EXPECT_EQ(events[1].operation, "dispatch");

    sink.clear();
    EXPECT_EQ(sink.size(), 0u);
    EXPECT_TRUE(sink.snapshot().empty());
}

TEST(BufferSinkTest, SnapshotReturnsCopy)
{
    BufferSink sink;

    sink.write(makeTraceEvent(7, "Kernel", "tick", "", 0));

    auto copy = sink.snapshot();
    ASSERT_EQ(copy.size(), 1u);

    copy.clear();
    EXPECT_EQ(sink.size(), 1u);
}
