/// @file test_round_robin.cpp
/// @brief Unit tests for Round Robin scheduling policy.

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/process/state.h"
#include "contur/scheduling/round_robin_policy.h"

using namespace contur;

TEST(RoundRobinPolicyTest, SelectsOldestReadyByLastStateChange)
{
    SimulationClock clock;
    RoundRobinPolicy policy(3);

    PCB p1(1, "p1");
    PCB p2(2, "p2");

    ASSERT_TRUE(p1.setState(ProcessState::Ready, 3));
    ASSERT_TRUE(p2.setState(ProcessState::Ready, 1));

    std::vector<SchedulingProcessSnapshot> ready = {
        {.pid = p1.id(), .lastStateChange = p1.timing().lastStateChange},
        {.pid = p2.id(), .lastStateChange = p2.timing().lastStateChange},
    };
    EXPECT_EQ(policy.selectNext(ready, clock), 2u);
}

TEST(RoundRobinPolicyTest, PreemptsWhenTimeSliceExpires)
{
    SimulationClock clock;
    RoundRobinPolicy policy(2);

    PCB running(1, "running");
    SchedulingProcessSnapshot candidate{.pid = 2};

    ASSERT_TRUE(running.setState(ProcessState::Ready, 0));
    ASSERT_TRUE(running.setState(ProcessState::Running, 1));

    clock.tick(); // now = 1
    clock.tick(); // now = 2
    clock.tick(); // now = 3

    SchedulingProcessSnapshot runningSnapshot{.pid = running.id(), .lastStateChange = running.timing().lastStateChange};
    EXPECT_TRUE(policy.shouldPreempt(runningSnapshot, candidate, clock));
}

TEST(RoundRobinPolicyTest, DoesNotPreemptBeforeTimeSlice)
{
    SimulationClock clock;
    RoundRobinPolicy policy(5);

    PCB running(1, "running");
    SchedulingProcessSnapshot candidate{.pid = 2};

    ASSERT_TRUE(running.setState(ProcessState::Ready, 0));
    ASSERT_TRUE(running.setState(ProcessState::Running, 1));

    clock.tick(); // now = 1
    clock.tick(); // now = 2

    SchedulingProcessSnapshot runningSnapshot{.pid = running.id(), .lastStateChange = running.timing().lastStateChange};
    EXPECT_FALSE(policy.shouldPreempt(runningSnapshot, candidate, clock));
}
