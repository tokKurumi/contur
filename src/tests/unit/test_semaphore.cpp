/// @file test_semaphore.cpp
/// @brief Unit tests for Semaphore synchronization primitive.

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/sync/semaphore.h"

using namespace contur;

TEST(SemaphoreTest, ConstructorClampsLimits)
{
    Semaphore sem(10, 0);

    EXPECT_EQ(sem.maxCount(), 1u);
    EXPECT_EQ(sem.count(), 1u);
    EXPECT_EQ(sem.waitingCount(), 0u);
    EXPECT_EQ(sem.name(), "Semaphore");
}

TEST(SemaphoreTest, AcquireDecrementsCount)
{
    Semaphore sem(2, 2);

    ASSERT_TRUE(sem.acquire(1).isOk());
    EXPECT_EQ(sem.count(), 1u);

    ASSERT_TRUE(sem.acquire(2).isOk());
    EXPECT_EQ(sem.count(), 0u);
}

TEST(SemaphoreTest, AcquireWhenEmptyQueuesWithoutDuplicates)
{
    Semaphore sem(1, 1);

    ASSERT_TRUE(sem.acquire(1).isOk());
    EXPECT_EQ(sem.count(), 0u);

    auto firstBusy = sem.acquire(2);
    auto duplicateBusy = sem.acquire(2);

    EXPECT_TRUE(firstBusy.isError());
    EXPECT_EQ(firstBusy.errorCode(), ErrorCode::ResourceBusy);
    EXPECT_TRUE(duplicateBusy.isError());
    EXPECT_EQ(duplicateBusy.errorCode(), ErrorCode::ResourceBusy);
    EXPECT_EQ(sem.waitingCount(), 1u);
    EXPECT_EQ(sem.count(), 0u);
}

TEST(SemaphoreTest, ReleaseWakesOneWaiterWithoutChangingCount)
{
    Semaphore sem(1, 1);

    ASSERT_TRUE(sem.acquire(1).isOk());
    ASSERT_TRUE(sem.acquire(2).isError());
    ASSERT_TRUE(sem.acquire(3).isError());
    EXPECT_EQ(sem.waitingCount(), 2u);
    EXPECT_EQ(sem.count(), 0u);

    ASSERT_TRUE(sem.release(1).isOk());

    EXPECT_EQ(sem.waitingCount(), 1u);
    EXPECT_EQ(sem.count(), 0u);
}

TEST(SemaphoreTest, ReleaseIncrementsCountUntilMax)
{
    Semaphore sem(0, 2);

    ASSERT_TRUE(sem.release(1).isOk());
    EXPECT_EQ(sem.count(), 1u);

    ASSERT_TRUE(sem.release(1).isOk());
    EXPECT_EQ(sem.count(), 2u);

    auto overflow = sem.release(1);

    EXPECT_TRUE(overflow.isError());
    EXPECT_EQ(overflow.errorCode(), ErrorCode::InvalidState);
    EXPECT_EQ(sem.count(), 2u);
}

TEST(SemaphoreTest, TryAcquireDoesNotEnqueueWhenBusy)
{
    Semaphore sem(0, 1);

    auto result = sem.tryAcquire(1);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::ResourceBusy);
    EXPECT_EQ(sem.waitingCount(), 0u);
}

TEST(SemaphoreTest, InvalidPidFailsForOperations)
{
    Semaphore sem(1, 1);

    auto acquire = sem.acquire(INVALID_PID);
    auto release = sem.release(INVALID_PID);
    auto tryAcquire = sem.tryAcquire(INVALID_PID);

    EXPECT_TRUE(acquire.isError());
    EXPECT_EQ(acquire.errorCode(), ErrorCode::InvalidPid);
    EXPECT_TRUE(release.isError());
    EXPECT_EQ(release.errorCode(), ErrorCode::InvalidPid);
    EXPECT_TRUE(tryAcquire.isError());
    EXPECT_EQ(tryAcquire.errorCode(), ErrorCode::InvalidPid);
}
