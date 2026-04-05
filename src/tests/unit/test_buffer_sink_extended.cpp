/// @file test_buffer_sink_extended.cpp
/// @brief Extended unit tests for BufferSink — move semantics, field
///        preservation, size tracking, write-after-clear, and empty state.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "contur/tracing/buffer_sink.h"
#include "contur/tracing/trace_event.h"
#include "contur/tracing/trace_level.h"

using namespace contur;

// Empty state
TEST(BufferSinkExtTest, DefaultConstructedSinkIsEmpty)
{
    BufferSink sink;
    EXPECT_EQ(sink.size(), 0u);
    EXPECT_TRUE(sink.snapshot().empty());
}

// Field preservation
TEST(BufferSinkExtTest, AllTraceEventFieldsArePreserved)
{
    BufferSink sink;

    TraceEvent ev = makeTraceEvent(42, "MySubsystem", "my-op", "detail=xyz", 3, TraceLevel::Warn);
    ev.workerId = 7;
    ev.sequence = 99;

    sink.write(ev);

    auto snap = sink.snapshot();
    ASSERT_EQ(snap.size(), 1u);

    const auto &stored = snap[0];
    EXPECT_EQ(stored.timestamp, 42u);
    EXPECT_EQ(stored.subsystem, "MySubsystem");
    EXPECT_EQ(stored.operation, "my-op");
    EXPECT_EQ(stored.details, "detail=xyz");
    EXPECT_EQ(stored.depth, 3u);
    EXPECT_EQ(stored.level, TraceLevel::Warn);
    EXPECT_EQ(stored.workerId, 7u);
    EXPECT_EQ(stored.sequence, 99u);
}

// Size tracking
TEST(BufferSinkExtTest, SizeIncrementsWithEachWrite)
{
    BufferSink sink;

    for (std::size_t i = 1; i <= 5; ++i)
    {
        sink.write(makeTraceEvent(i, "A", "op", "", 0));
        EXPECT_EQ(sink.size(), i);
    }
}

TEST(BufferSinkExtTest, ClearResetsSize)
{
    BufferSink sink;
    sink.write(makeTraceEvent(1, "A", "op", "", 0));
    sink.write(makeTraceEvent(2, "A", "op", "", 0));
    ASSERT_EQ(sink.size(), 2u);

    sink.clear();
    EXPECT_EQ(sink.size(), 0u);
}

TEST(BufferSinkExtTest, WriteAfterClearAccumulatesFromZero)
{
    BufferSink sink;
    sink.write(makeTraceEvent(1, "A", "op1", "", 0));
    sink.clear();

    sink.write(makeTraceEvent(10, "B", "op2", "new", 0));
    EXPECT_EQ(sink.size(), 1u);

    auto snap = sink.snapshot();
    ASSERT_EQ(snap.size(), 1u);
    EXPECT_EQ(snap[0].timestamp, 10u);
    EXPECT_EQ(snap[0].operation, "op2");
}

// Snapshot ordering
TEST(BufferSinkExtTest, SnapshotPreservesInsertionOrder)
{
    BufferSink sink;
    for (std::size_t i = 0; i < 10; ++i)
    {
        sink.write(makeTraceEvent(i, "S", "op", std::to_string(i), 0));
    }

    auto snap = sink.snapshot();
    ASSERT_EQ(snap.size(), 10u);
    for (std::size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(snap[i].timestamp, i) << "entry " << i;
    }
}

// Move semantics
TEST(BufferSinkExtTest, MoveConstructedSinkOwnsEvents)
{
    BufferSink original;
    original.write(makeTraceEvent(1, "S", "a", "", 0));
    original.write(makeTraceEvent(2, "S", "b", "", 0));

    BufferSink moved(std::move(original));

    EXPECT_EQ(moved.size(), 2u);
    auto snap = moved.snapshot();
    EXPECT_EQ(snap[0].operation, "a");
    EXPECT_EQ(snap[1].operation, "b");
}

TEST(BufferSinkExtTest, MoveAssignedSinkOwnsEvents)
{
    BufferSink src;
    src.write(makeTraceEvent(5, "X", "go", "d=1", 1));

    BufferSink dst;
    dst = std::move(src);

    EXPECT_EQ(dst.size(), 1u);
    EXPECT_EQ(dst.snapshot()[0].details, "d=1");
}

// Multiple levels
TEST(BufferSinkExtTest, AllTraceLevelsStoredWithoutFiltering)
{
    // BufferSink itself has no filter — it stores everything.
    BufferSink sink;
    sink.write(makeTraceEvent(0, "S", "debug-op", "", 0, TraceLevel::Debug));
    sink.write(makeTraceEvent(1, "S", "info-op", "", 0, TraceLevel::Info));
    sink.write(makeTraceEvent(2, "S", "warn-op", "", 0, TraceLevel::Warn));
    sink.write(makeTraceEvent(3, "S", "error-op", "", 0, TraceLevel::Error));

    EXPECT_EQ(sink.size(), 4u);
    auto snap = sink.snapshot();
    EXPECT_EQ(snap[0].level, TraceLevel::Debug);
    EXPECT_EQ(snap[1].level, TraceLevel::Info);
    EXPECT_EQ(snap[2].level, TraceLevel::Warn);
    EXPECT_EQ(snap[3].level, TraceLevel::Error);
}

// Independent snapshots
TEST(BufferSinkExtTest, TwoSnapshotsAreIndependentCopies)
{
    BufferSink sink;
    sink.write(makeTraceEvent(1, "S", "op", "", 0));

    auto s1 = sink.snapshot();
    sink.write(makeTraceEvent(2, "S", "op2", "", 0));
    auto s2 = sink.snapshot();

    EXPECT_EQ(s1.size(), 1u);
    EXPECT_EQ(s2.size(), 2u);
}
