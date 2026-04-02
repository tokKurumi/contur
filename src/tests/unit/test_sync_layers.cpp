/// @file test_sync_layers.cpp
/// @brief Tests for synchronization layer split contracts.

#include <memory>

#include <gtest/gtest.h>

#include "contur/sync/critical_section.h"
#include "contur/sync/mutex.h"
#include "contur/sync/semaphore.h"

using namespace contur;

namespace {

    class FakeKernelInternalPrimitive final : public ISyncPrimitive
    {
        public:
        [[nodiscard]] Result<void> acquire(ProcessId pid) override
        {
            return pid == INVALID_PID ? Result<void>::error(ErrorCode::InvalidPid) : Result<void>::ok();
        }

        [[nodiscard]] Result<void> release(ProcessId pid) override
        {
            return pid == INVALID_PID ? Result<void>::error(ErrorCode::InvalidPid) : Result<void>::ok();
        }

        [[nodiscard]] Result<void> tryAcquire(ProcessId pid) override
        {
            return acquire(pid);
        }

        [[nodiscard]] std::string_view name() const noexcept override
        {
            return "FakeKernelInternalPrimitive";
        }

        [[nodiscard]] SyncLayer layer() const noexcept override
        {
            return SyncLayer::KernelInternal;
        }
    };

} // namespace

TEST(SyncLayerTest, MutexAndSemaphoreAreSimulatedResourceLayer)
{
    Mutex mutex;
    Semaphore semaphore(1, 1);

    EXPECT_EQ(mutex.layer(), SyncLayer::SimulatedResource);
    EXPECT_EQ(semaphore.layer(), SyncLayer::SimulatedResource);
}

TEST(SyncLayerTest, CriticalSectionDefaultLayerIsSimulatedResource)
{
    CriticalSection cs;

    EXPECT_EQ(cs.layer(), SyncLayer::SimulatedResource);
}

TEST(SyncLayerTest, CriticalSectionForwardsInjectedPrimitiveLayer)
{
    auto primitive = std::make_unique<FakeKernelInternalPrimitive>();
    CriticalSection cs(std::move(primitive));

    EXPECT_EQ(cs.layer(), SyncLayer::KernelInternal);
}
