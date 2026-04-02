/// @file test_deadlock_detector_concurrent.cpp
/// @brief Concurrent/thread-aware deadlock detector tests.

#include <algorithm>

#include <gtest/gtest.h>

#include "contur/sync/deadlock_detector.h"

using namespace contur;

TEST(DeadlockDetectorConcurrentTest, ThreadAwareWaitForCycleIsDetected)
{
    DeadlockDetector detector;

    detector.onAcquire(1, 100, 11);
    detector.onAcquire(2, 200, 22);
    detector.onWait(1, 200, 11);
    detector.onWait(2, 100, 22);

    EXPECT_TRUE(detector.hasDeadlock());

    auto deadlocked = detector.getDeadlockedProcesses();
    std::sort(deadlocked.begin(), deadlocked.end());
    ASSERT_EQ(deadlocked.size(), 2u);
    EXPECT_EQ(deadlocked[0], 1u);
    EXPECT_EQ(deadlocked[1], 2u);
}

TEST(DeadlockDetectorConcurrentTest, SameProcessDifferentThreadsCanFormCycle)
{
    DeadlockDetector detector;

    detector.onAcquire(1, 100, 1);
    detector.onAcquire(1, 200, 2);
    detector.onWait(1, 200, 1);
    detector.onWait(1, 100, 2);

    EXPECT_TRUE(detector.hasDeadlock());

    auto deadlocked = detector.getDeadlockedProcesses();
    ASSERT_EQ(deadlocked.size(), 1u);
    EXPECT_EQ(deadlocked[0], 1u);
}

TEST(DeadlockDetectorConcurrentTest, InternalLockOrderCycleIsDetected)
{
    DeadlockDetector detector;

    detector.onInternalLockAcquire(1, 10);
    detector.onInternalLockAcquire(1, 20); // edge 10 -> 20

    detector.onInternalLockAcquire(2, 20);
    detector.onInternalLockAcquire(2, 10); // edge 20 -> 10

    EXPECT_TRUE(detector.hasInternalLockOrderCycle());
    EXPECT_TRUE(detector.hasDeadlock());

    auto locks = detector.getInternalLockOrderCycle();
    std::sort(locks.begin(), locks.end());
    ASSERT_EQ(locks.size(), 2u);
    EXPECT_EQ(locks[0], 10u);
    EXPECT_EQ(locks[1], 20u);
}

TEST(DeadlockDetectorConcurrentTest, InternalLockOrderWithoutCycleIsSafe)
{
    DeadlockDetector detector;

    detector.onInternalLockAcquire(1, 10);
    detector.onInternalLockAcquire(1, 20);
    detector.onInternalLockRelease(1, 20);
    detector.onInternalLockRelease(1, 10);

    detector.onInternalLockAcquire(2, 10);
    detector.onInternalLockAcquire(2, 20);

    EXPECT_FALSE(detector.hasInternalLockOrderCycle());
    EXPECT_FALSE(detector.hasDeadlock());
}
