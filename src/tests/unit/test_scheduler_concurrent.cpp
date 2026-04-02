/// @file test_scheduler_concurrent.cpp
/// @brief Unit tests for lane-aware concurrent scheduler behavior.

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/scheduling/fcfs_policy.h"
#include "contur/scheduling/scheduler.h"

using namespace contur;

TEST(SchedulerConcurrentTest, ConfigureLanesValidatesArgumentsAndState)
{
    Scheduler scheduler(std::make_unique<FcfsPolicy>());

    auto zero = scheduler.configureLanes(0);
    ASSERT_TRUE(zero.isError());
    EXPECT_EQ(zero.errorCode(), ErrorCode::InvalidArgument);

    PCB p1(1, "p1");
    ASSERT_TRUE(scheduler.enqueue(p1, 0).isOk());

    auto busy = scheduler.configureLanes(2);
    ASSERT_TRUE(busy.isError());
    EXPECT_EQ(busy.errorCode(), ErrorCode::InvalidState);
}

TEST(SchedulerConcurrentTest, EnqueueToLaneCreatesIndependentPerLaneQueues)
{
    Scheduler scheduler(std::make_unique<FcfsPolicy>());
    ASSERT_TRUE(scheduler.configureLanes(3).isOk());

    PCB p1(1, "p1");
    PCB p2(2, "p2");
    PCB p3(3, "p3");

    ASSERT_TRUE(scheduler.enqueueToLane(p1, 0, 0).isOk());
    ASSERT_TRUE(scheduler.enqueueToLane(p2, 1, 0).isOk());
    ASSERT_TRUE(scheduler.enqueueToLane(p3, 2, 0).isOk());

    auto lanes = scheduler.getPerLaneQueueSnapshot();
    ASSERT_EQ(lanes.size(), 3u);
    ASSERT_EQ(lanes[0].size(), 1u);
    ASSERT_EQ(lanes[1].size(), 1u);
    ASSERT_EQ(lanes[2].size(), 1u);
    EXPECT_EQ(lanes[0][0], 1u);
    EXPECT_EQ(lanes[1][0], 2u);
    EXPECT_EQ(lanes[2][0], 3u);
}

TEST(SchedulerConcurrentTest, SelectNextForLaneTracksMultipleRunningProcesses)
{
    SimulationClock clock;
    Scheduler scheduler(std::make_unique<FcfsPolicy>());
    ASSERT_TRUE(scheduler.configureLanes(2).isOk());

    PCB p1(1, "p1");
    PCB p2(2, "p2");

    ASSERT_TRUE(scheduler.enqueueToLane(p1, 0, 0).isOk());
    ASSERT_TRUE(scheduler.enqueueToLane(p2, 1, 0).isOk());

    auto lane0 = scheduler.selectNextForLane(0, clock);
    auto lane1 = scheduler.selectNextForLane(1, clock);

    ASSERT_TRUE(lane0.isOk());
    ASSERT_TRUE(lane1.isOk());
    EXPECT_EQ(lane0.value(), 1u);
    EXPECT_EQ(lane1.value(), 2u);

    auto running = scheduler.runningProcesses();
    std::sort(running.begin(), running.end());
    ASSERT_EQ(running.size(), 2u);
    EXPECT_EQ(running[0], 1u);
    EXPECT_EQ(running[1], 2u);
}

TEST(SchedulerConcurrentTest, StealMovesWorkToThiefLaneAndMaintainsUniqueness)
{
    SimulationClock clock;
    Scheduler scheduler(std::make_unique<FcfsPolicy>());
    ASSERT_TRUE(scheduler.configureLanes(2).isOk());

    PCB p1(1, "p1");
    PCB p2(2, "p2");

    ASSERT_TRUE(scheduler.enqueueToLane(p1, 0, 0).isOk());
    ASSERT_TRUE(scheduler.enqueueToLane(p2, 0, 0).isOk());

    auto stolen = scheduler.stealNextForLane(1, clock);
    ASSERT_TRUE(stolen.isOk());

    auto lanes = scheduler.getPerLaneQueueSnapshot();
    ASSERT_EQ(lanes.size(), 2u);

    auto running = scheduler.runningProcesses();
    ASSERT_EQ(running.size(), 1u);

    std::vector<ProcessId> all;
    for (const auto &lane : lanes)
    {
        all.insert(all.end(), lane.begin(), lane.end());
    }
    all.insert(all.end(), running.begin(), running.end());

    std::sort(all.begin(), all.end());
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0], 1u);
    EXPECT_EQ(all[1], 2u);
}
