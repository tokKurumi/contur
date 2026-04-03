/// @file test_tui_history_buffer.cpp
/// @brief Unit tests for TUI history buffer cursor semantics.

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/tui/history_buffer.h"

using namespace contur;

namespace {

    TuiHistoryEntry makeEntry(std::size_t sequence, Tick tick)
    {
        TuiHistoryEntry entry;
        entry.sequence = sequence;
        entry.snapshot.sequence = static_cast<std::uint64_t>(sequence);
        entry.snapshot.currentTick = tick;
        return entry;
    }

} // namespace

TEST(TuiHistoryBufferTest, AppendedEntryBecomesCurrentAndLatest)
{
    HistoryBuffer history(4);

    ASSERT_TRUE(history.append(makeEntry(1, 10)).isOk());

    auto current = history.current();
    auto latest = history.latest();

    ASSERT_FALSE(history.empty());
    ASSERT_TRUE(current.has_value());
    ASSERT_TRUE(latest.has_value());

    EXPECT_EQ(history.size(), 1u);
    EXPECT_EQ(history.cursor(), 0u);
    EXPECT_EQ(current->get().sequence, 1u);
    EXPECT_EQ(latest->get().snapshot.currentTick, 10u);
}

TEST(TuiHistoryBufferTest, SeekBackwardAndForwardMovesCursor)
{
    HistoryBuffer history(6);
    ASSERT_TRUE(history.append(makeEntry(1, 10)).isOk());
    ASSERT_TRUE(history.append(makeEntry(2, 20)).isOk());
    ASSERT_TRUE(history.append(makeEntry(3, 30)).isOk());

    ASSERT_TRUE(history.seekBackward(2).isOk());
    EXPECT_EQ(history.cursor(), 0u);
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().snapshot.currentTick, 10u);

    ASSERT_TRUE(history.seekForward(1).isOk());
    EXPECT_EQ(history.cursor(), 1u);
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().snapshot.currentTick, 20u);
}

TEST(TuiHistoryBufferTest, SeekBeyondBoundsReturnsNotFound)
{
    HistoryBuffer history(4);
    ASSERT_TRUE(history.append(makeEntry(1, 11)).isOk());

    auto backward = history.seekBackward(2);
    ASSERT_TRUE(backward.isError());
    EXPECT_EQ(backward.errorCode(), ErrorCode::NotFound);

    auto forward = history.seekForward(1);
    ASSERT_TRUE(forward.isError());
    EXPECT_EQ(forward.errorCode(), ErrorCode::NotFound);
}

TEST(TuiHistoryBufferTest, BoundedCapacityDropsOldestEntry)
{
    HistoryBuffer history(3);
    ASSERT_TRUE(history.append(makeEntry(1, 10)).isOk());
    ASSERT_TRUE(history.append(makeEntry(2, 20)).isOk());
    ASSERT_TRUE(history.append(makeEntry(3, 30)).isOk());

    ASSERT_TRUE(history.seekBackward(2).isOk());
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().sequence, 1u);

    ASSERT_TRUE(history.append(makeEntry(4, 40)).isOk());

    EXPECT_EQ(history.size(), 3u);
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().sequence, 4u);

    ASSERT_TRUE(history.seekBackward(2).isOk());
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().sequence, 2u);
}

TEST(TuiHistoryBufferTest, AppendAfterSeekMovesCursorToNewest)
{
    HistoryBuffer history(5);
    ASSERT_TRUE(history.append(makeEntry(1, 10)).isOk());
    ASSERT_TRUE(history.append(makeEntry(2, 20)).isOk());
    ASSERT_TRUE(history.append(makeEntry(3, 30)).isOk());

    ASSERT_TRUE(history.seekBackward(2).isOk());
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().sequence, 1u);

    ASSERT_TRUE(history.append(makeEntry(4, 40)).isOk());

    EXPECT_EQ(history.cursor(), history.size() - 1);
    ASSERT_TRUE(history.current().has_value());
    EXPECT_EQ(history.current()->get().sequence, 4u);
}
