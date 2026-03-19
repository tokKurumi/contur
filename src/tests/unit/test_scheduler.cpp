/// @file test_scheduler.cpp
/// @brief Unit tests for scheduler queue/state transitions.

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/process/state.h"
#include "contur/scheduling/fcfs_policy.h"
#include "contur/scheduling/scheduler.h"

using namespace contur;

TEST(SchedulerTest, EnqueueAndSelectMovesProcessToRunning)
{
    SimulationClock clock;
    Scheduler scheduler(std::make_unique<FcfsPolicy>());

    PCB p1(1, "p1");
    ASSERT_TRUE(scheduler.enqueue(p1, 1).isOk());

    auto selected = scheduler.selectNext(clock);
    ASSERT_TRUE(selected.isOk());
    EXPECT_EQ(selected.value(), 1u);
    EXPECT_EQ(scheduler.runningProcess(), 1u);
    EXPECT_EQ(p1.state(), ProcessState::Running);
}

TEST(SchedulerTest, BlockAndUnblockTransitions)
{
    SimulationClock clock;
    Scheduler scheduler(std::make_unique<FcfsPolicy>());

    PCB p1(1, "p1");
    ASSERT_TRUE(scheduler.enqueue(p1, 0).isOk());
    ASSERT_TRUE(scheduler.selectNext(clock).isOk());

    ASSERT_TRUE(scheduler.blockRunning(5).isOk());
    EXPECT_EQ(scheduler.runningProcess(), INVALID_PID);
    EXPECT_EQ(p1.state(), ProcessState::Blocked);

    ASSERT_TRUE(scheduler.unblock(1, 7).isOk());
    EXPECT_EQ(p1.state(), ProcessState::Ready);
    ASSERT_EQ(scheduler.getQueueSnapshot().size(), 1u);
}

TEST(SchedulerTest, TerminateRunningProcess)
{
    SimulationClock clock;
    Scheduler scheduler(std::make_unique<FcfsPolicy>());

    PCB p1(1, "p1");
    ASSERT_TRUE(scheduler.enqueue(p1, 0).isOk());
    ASSERT_TRUE(scheduler.selectNext(clock).isOk());

    ASSERT_TRUE(scheduler.terminate(1, 10).isOk());
    EXPECT_EQ(p1.state(), ProcessState::Terminated);
    EXPECT_EQ(scheduler.runningProcess(), INVALID_PID);
}
