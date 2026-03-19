/// @file test_mutex.cpp
/// @brief Unit tests for Mutex synchronization primitive.

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/sync/mutex.h"

using namespace contur;

TEST(MutexTest, InitialStateIsUnlocked)
{
    Mutex mutex;

    EXPECT_FALSE(mutex.isLocked());
    EXPECT_FALSE(mutex.owner().has_value());
    EXPECT_EQ(mutex.recursionDepth(), 0u);
    EXPECT_EQ(mutex.waitingCount(), 0u);
    EXPECT_EQ(mutex.name(), "Mutex");
}

TEST(MutexTest, AcquireWithInvalidPidFails)
{
    Mutex mutex;

    auto result = mutex.acquire(INVALID_PID);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::InvalidPid);
    EXPECT_FALSE(mutex.isLocked());
}

TEST(MutexTest, AcquireAndReleaseByOwner)
{
    Mutex mutex;

    ASSERT_TRUE(mutex.acquire(1).isOk());
    EXPECT_TRUE(mutex.isLocked());
    ASSERT_TRUE(mutex.owner().has_value());
    EXPECT_EQ(mutex.owner().value(), 1u);
    EXPECT_EQ(mutex.recursionDepth(), 1u);

    auto releaseResult = mutex.release(1);

    EXPECT_TRUE(releaseResult.isOk());
    EXPECT_FALSE(mutex.isLocked());
    EXPECT_FALSE(mutex.owner().has_value());
    EXPECT_EQ(mutex.recursionDepth(), 0u);
}

TEST(MutexTest, RecursiveAcquireIncrementsDepth)
{
    Mutex mutex;

    ASSERT_TRUE(mutex.acquire(1).isOk());
    ASSERT_TRUE(mutex.acquire(1).isOk());
    ASSERT_TRUE(mutex.tryAcquire(1).isOk());

    EXPECT_TRUE(mutex.isLocked());
    ASSERT_TRUE(mutex.owner().has_value());
    EXPECT_EQ(mutex.owner().value(), 1u);
    EXPECT_EQ(mutex.recursionDepth(), 3u);

    ASSERT_TRUE(mutex.release(1).isOk());
    EXPECT_EQ(mutex.recursionDepth(), 2u);
    ASSERT_TRUE(mutex.release(1).isOk());
    EXPECT_EQ(mutex.recursionDepth(), 1u);
    ASSERT_TRUE(mutex.release(1).isOk());
    EXPECT_FALSE(mutex.isLocked());
}

TEST(MutexTest, NonOwnerReleaseFails)
{
    Mutex mutex;

    ASSERT_TRUE(mutex.acquire(10).isOk());

    auto result = mutex.release(99);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::PermissionDenied);
    ASSERT_TRUE(mutex.owner().has_value());
    EXPECT_EQ(mutex.owner().value(), 10u);
    EXPECT_EQ(mutex.recursionDepth(), 1u);
}

TEST(MutexTest, ReleaseWithoutOwnerFails)
{
    Mutex mutex;

    auto result = mutex.release(1);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::InvalidState);
}

TEST(MutexTest, ContentionAddsToWaitQueueWithoutDuplicates)
{
    Mutex mutex;

    ASSERT_TRUE(mutex.acquire(1).isOk());

    auto firstBusy = mutex.acquire(2);
    auto duplicateBusy = mutex.acquire(2);

    EXPECT_TRUE(firstBusy.isError());
    EXPECT_EQ(firstBusy.errorCode(), ErrorCode::ResourceBusy);
    EXPECT_TRUE(duplicateBusy.isError());
    EXPECT_EQ(duplicateBusy.errorCode(), ErrorCode::ResourceBusy);
    EXPECT_EQ(mutex.waitingCount(), 1u);
}

TEST(MutexTest, ReleaseHandsOwnershipToFirstWaitingProcess)
{
    Mutex mutex;

    ASSERT_TRUE(mutex.acquire(1).isOk());
    ASSERT_TRUE(mutex.acquire(2).isError());
    ASSERT_TRUE(mutex.acquire(3).isError());
    EXPECT_EQ(mutex.waitingCount(), 2u);

    ASSERT_TRUE(mutex.release(1).isOk());

    ASSERT_TRUE(mutex.owner().has_value());
    EXPECT_EQ(mutex.owner().value(), 2u);
    EXPECT_EQ(mutex.recursionDepth(), 1u);
    EXPECT_EQ(mutex.waitingCount(), 1u);
}

TEST(MutexTest, TryAcquireDoesNotEnqueueWaiters)
{
    Mutex mutex;

    ASSERT_TRUE(mutex.tryAcquire(1).isOk());

    auto busy = mutex.tryAcquire(2);

    EXPECT_TRUE(busy.isError());
    EXPECT_EQ(busy.errorCode(), ErrorCode::ResourceBusy);
    EXPECT_EQ(mutex.waitingCount(), 0u);
}
