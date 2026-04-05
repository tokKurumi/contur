/// @file test_kernel_diagnostics_extended.cpp
/// @brief Extended unit tests for KernelDiagnostics — empty kernel snapshot,
///        zero processes, field consistency, repeated captures, process detail.

#include <optional>
#include <span>
#include <string>

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/kernel/i_kernel.h"
#include "contur/kernel/kernel_diagnostics.h"
#include "contur/sync/i_sync_primitive.h"

using namespace contur;

namespace {

    /// Minimal IKernel fake: returns a mutable snapshot.
    class ConfigurableKernelFake final : public IKernel
    {
        public:
        KernelSnapshot snap;

        [[nodiscard]] Result<ProcessId> createProcess(const ProcessConfig &) override
        {
            return Result<ProcessId>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> terminateProcess(ProcessId) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> tick(std::size_t) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> runForTicks(std::size_t, std::size_t) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<RegisterValue> syscall(ProcessId, SyscallId, std::span<const RegisterValue>) override
        {
            return Result<RegisterValue>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> registerSyscallHandler(SyscallId, SyscallHandlerFn) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> registerSyncPrimitive(const std::string &, std::unique_ptr<ISyncPrimitive>) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> enterCritical(ProcessId, std::string_view) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] Result<void> leaveCritical(ProcessId, std::string_view) override
        {
            return Result<void>::error(ErrorCode::NotImplemented);
        }
        [[nodiscard]] KernelSnapshot snapshot() const override
        {
            return snap;
        }
        [[nodiscard]] Tick now() const noexcept override
        {
            return snap.currentTick;
        }
        [[nodiscard]] bool hasProcess(ProcessId) const noexcept override
        {
            return false;
        }
        [[nodiscard]] std::size_t processCount() const noexcept override
        {
            return snap.processCount;
        }
    };

} // namespace

// Empty / zero-state snapshot
TEST(KernelDiagnosticsExtTest, EmptySnapshotReturnsZeroFields)
{
    ConfigurableKernelFake kernel;
    // snap is default-initialised: all zeros / empty collections
    KernelDiagnostics diag(kernel);

    auto r = diag.captureSnapshot();
    ASSERT_TRUE(r.isOk());

    const auto &k = r.value().kernel;
    EXPECT_EQ(k.currentTick, 0u);
    EXPECT_EQ(k.processCount, 0u);
    EXPECT_EQ(k.readyCount, 0u);
    EXPECT_EQ(k.blockedCount, 0u);
    EXPECT_TRUE(k.runningPids.empty());
    EXPECT_TRUE(k.readyQueue.empty());
    EXPECT_TRUE(k.blockedQueue.empty());
    EXPECT_TRUE(k.processes.empty());
}

// Repeated captures reflect current kernel state
TEST(KernelDiagnosticsExtTest, RepeatedCaptureReflectsUpdatedSnapshot)
{
    ConfigurableKernelFake kernel;
    KernelDiagnostics diag(kernel);

    // First capture — tick=0
    auto r0 = diag.captureSnapshot();
    ASSERT_TRUE(r0.isOk());
    EXPECT_EQ(r0.value().kernel.currentTick, 0u);

    // Mutate the fake kernel state
    kernel.snap.currentTick = 42;
    kernel.snap.processCount = 3;

    auto r1 = diag.captureSnapshot();
    ASSERT_TRUE(r1.isOk());
    EXPECT_EQ(r1.value().kernel.currentTick, 42u);
    EXPECT_EQ(r1.value().kernel.processCount, 3u);
}

// Process list fields propagated correctly
TEST(KernelDiagnosticsExtTest, ProcessSnapshotFieldsArePropagatedVerbatim)
{
    ConfigurableKernelFake kernel;
    kernel.snap.processCount = 2;
    kernel.snap.processes = {
        KernelProcessSnapshot{
            .id = 5, .name = "alpha", .state = ProcessState::Running, .cpuTime = 10, .laneIndex = std::nullopt
        },
        KernelProcessSnapshot{
            .id = 6, .name = "beta", .state = ProcessState::Blocked, .cpuTime = 3, .laneIndex = std::nullopt
        },
    };

    KernelDiagnostics diag(kernel);
    auto r = diag.captureSnapshot();
    ASSERT_TRUE(r.isOk());

    const auto &procs = r.value().kernel.processes;
    ASSERT_EQ(procs.size(), 2u);
    EXPECT_EQ(procs[0].id, 5u);
    EXPECT_EQ(procs[0].name, "alpha");
    EXPECT_EQ(procs[0].state, ProcessState::Running);
    EXPECT_EQ(procs[0].cpuTime, 10u);
    EXPECT_EQ(procs[1].id, 6u);
    EXPECT_EQ(procs[1].name, "beta");
    EXPECT_EQ(procs[1].state, ProcessState::Blocked);
}

// Queue snapshots
TEST(KernelDiagnosticsExtTest, QueueSnapshotsReflectKernelState)
{
    ConfigurableKernelFake kernel;
    kernel.snap.readyQueue = {1, 2, 3};
    kernel.snap.blockedQueue = {4};
    kernel.snap.runningPids = {5};
    kernel.snap.readyCount = 3;
    kernel.snap.blockedCount = 1;

    KernelDiagnostics diag(kernel);
    auto r = diag.captureSnapshot();
    ASSERT_TRUE(r.isOk());

    const auto &k = r.value().kernel;
    EXPECT_EQ(k.readyQueue, (std::vector<ProcessId>{1, 2, 3}));
    EXPECT_EQ(k.blockedQueue, (std::vector<ProcessId>{4}));
    EXPECT_EQ(k.runningPids, (std::vector<ProcessId>{5}));
    EXPECT_EQ(k.readyCount, 3u);
    EXPECT_EQ(k.blockedCount, 1u);
}

// Memory stats propagated
TEST(KernelDiagnosticsExtTest, MemoryStatsArePropagated)
{
    ConfigurableKernelFake kernel;
    kernel.snap.totalVirtualSlots = 64;
    kernel.snap.freeVirtualSlots = 48;
    kernel.snap.totalFrames = 128;
    kernel.snap.freeFrames = 100;

    KernelDiagnostics diag(kernel);
    auto r = diag.captureSnapshot();
    ASSERT_TRUE(r.isOk());

    const auto &k = r.value().kernel;
    EXPECT_EQ(k.totalVirtualSlots, 64u);
    EXPECT_EQ(k.freeVirtualSlots, 48u);
    ASSERT_TRUE(k.totalFrames.has_value());
    ASSERT_TRUE(k.freeFrames.has_value());
    EXPECT_EQ(k.totalFrames.value(), 128u);
    EXPECT_EQ(k.freeFrames.value(), 100u);
}

// Policy name propagated
TEST(KernelDiagnosticsExtTest, PolicyNameIsPropagated)
{
    ConfigurableKernelFake kernel;
    kernel.snap.policyName = "RoundRobin";

    KernelDiagnostics diag(kernel);
    auto r = diag.captureSnapshot();
    ASSERT_TRUE(r.isOk());
    EXPECT_EQ(r.value().kernel.policyName, "RoundRobin");
}

// Per-lane queues
TEST(KernelDiagnosticsExtTest, PerLaneQueuesArePropagated)
{
    ConfigurableKernelFake kernel;
    kernel.snap.perLaneReadyQueues = {{1, 2}, {3}, {}};

    KernelDiagnostics diag(kernel);
    auto r = diag.captureSnapshot();
    ASSERT_TRUE(r.isOk());

    const auto &lanes = r.value().kernel.perLaneReadyQueues;
    ASSERT_EQ(lanes.size(), 3u);
    EXPECT_EQ(lanes[0], (std::vector<ProcessId>{1, 2}));
    EXPECT_EQ(lanes[1], (std::vector<ProcessId>{3}));
    EXPECT_TRUE(lanes[2].empty());
}
