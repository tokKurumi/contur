/// @file test_deadlock_detector.cpp
/// @brief Unit tests for DeadlockDetector and Banker's safety check.

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "contur/sync/deadlock_detector.h"

using namespace contur;

TEST(DeadlockDetectorTest, NoDeadlockInitially)
{
    DeadlockDetector detector;

    EXPECT_FALSE(detector.hasDeadlock());
    EXPECT_TRUE(detector.getDeadlockedProcesses().empty());
}

TEST(DeadlockDetectorTest, DetectsTwoProcessCycle)
{
    DeadlockDetector detector;

    detector.onAcquire(1, 100);
    detector.onAcquire(2, 200);
    detector.onWait(1, 200);
    detector.onWait(2, 100);

    EXPECT_TRUE(detector.hasDeadlock());

    auto deadlocked = detector.getDeadlockedProcesses();
    std::sort(deadlocked.begin(), deadlocked.end());
    ASSERT_EQ(deadlocked.size(), 2u);
    EXPECT_EQ(deadlocked[0], 1u);
    EXPECT_EQ(deadlocked[1], 2u);
}

TEST(DeadlockDetectorTest, WaitChainWithoutCycleIsNotDeadlock)
{
    DeadlockDetector detector;

    detector.onAcquire(2, 100);
    detector.onAcquire(3, 200);
    detector.onWait(1, 100); // 1 -> 2
    detector.onWait(2, 200); // 2 -> 3

    EXPECT_FALSE(detector.hasDeadlock());
    EXPECT_TRUE(detector.getDeadlockedProcesses().empty());
}

TEST(DeadlockDetectorTest, AcquireClearsOutgoingWaitEdges)
{
    DeadlockDetector detector;

    detector.onAcquire(1, 10);
    detector.onAcquire(2, 20);
    detector.onWait(1, 20); // 1 -> 2

    detector.onAcquire(1, 30); // clear outgoing for 1

    EXPECT_FALSE(detector.hasDeadlock());
}

TEST(DeadlockDetectorTest, SelfWaitEdgeIsIgnored)
{
    DeadlockDetector detector;

    detector.onAcquire(1, 10);
    detector.onWait(1, 10);

    EXPECT_FALSE(detector.hasDeadlock());
    EXPECT_TRUE(detector.getDeadlockedProcesses().empty());
}

TEST(DeadlockDetectorTest, WaitingForUnownedResourceCreatesNoEdge)
{
    DeadlockDetector detector;

    detector.onWait(1, 777);

    EXPECT_FALSE(detector.hasDeadlock());
}

TEST(DeadlockDetectorTest, BankerReturnsFalseForMismatchedProcessCount)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {1, 0}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {1, 0}},
        {2, {0, 1}},
    };
    std::vector<std::uint32_t> available = {0, 0};

    EXPECT_FALSE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerReturnsFalseForInvalidRows)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {INVALID_PID, {1}},
        {2, {0}},
    };
    std::vector<ResourceAllocation> maximum = {
        {INVALID_PID, {1}},
        {2, {1}},
    };
    std::vector<std::uint32_t> available = {0};

    EXPECT_FALSE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerReturnsFalseForDuplicatePid)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {0, 1}},
        {1, {1, 0}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {1, 1}},
        {1, {1, 1}},
    };
    std::vector<std::uint32_t> available = {0, 0};

    EXPECT_FALSE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerReturnsFalseWhenCurrentExceedsMaximum)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {2}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {1}},
    };
    std::vector<std::uint32_t> available = {0};

    EXPECT_FALSE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerReturnsFalseForAvailableDimensionMismatch)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {1, 0}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {1, 1}},
    };
    std::vector<std::uint32_t> available = {0};

    EXPECT_FALSE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerReturnsTrueForSafeState)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {1}},
        {2, {0}},
        {3, {1}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {3}},
        {2, {2}},
        {3, {1}},
    };
    std::vector<std::uint32_t> available = {1};

    EXPECT_TRUE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerReturnsFalseForUnsafeState)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {1}},
        {2, {1}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {2}},
        {2, {2}},
    };
    std::vector<std::uint32_t> available = {0};

    EXPECT_FALSE(detector.isSafeState(current, maximum, available));
}

TEST(DeadlockDetectorTest, BankerTreatsEmptyInputAsSafe)
{
    DeadlockDetector detector;
    std::vector<std::uint32_t> available = {1};

    EXPECT_TRUE(detector.isSafeState({}, {}, available));
}

TEST(DeadlockDetectorTest, BankerTreatsZeroResourceDimensionAsSafe)
{
    DeadlockDetector detector;

    std::vector<ResourceAllocation> current = {
        {1, {}},
        {2, {}},
    };
    std::vector<ResourceAllocation> maximum = {
        {1, {}},
        {2, {}},
    };
    std::vector<std::uint32_t> available;

    EXPECT_TRUE(detector.isSafeState(current, maximum, available));
}
