/// @file test_fcfs.cpp
/// @brief Unit tests for FCFS scheduling policy.

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/scheduling/fcfs_policy.h"

using namespace contur;

TEST(FcfsPolicyTest, SelectsEarliestArrival)
{
    SimulationClock clock;
    FcfsPolicy policy;

    PCB p1(1, "p1", Priority{}, 10);
    PCB p2(2, "p2", Priority{}, 5);
    PCB p3(3, "p3", Priority{}, 20);

    std::vector<SchedulingProcessSnapshot> ready = {
        {.pid = p1.id(), .arrivalTime = p1.timing().arrivalTime},
        {.pid = p2.id(), .arrivalTime = p2.timing().arrivalTime},
        {.pid = p3.id(), .arrivalTime = p3.timing().arrivalTime},
    };
    EXPECT_EQ(policy.selectNext(ready, clock), 2u);
}

TEST(FcfsPolicyTest, ShouldNotPreempt)
{
    SimulationClock clock;
    FcfsPolicy policy;

    SchedulingProcessSnapshot running{.pid = 1};
    SchedulingProcessSnapshot candidate{.pid = 2};
    EXPECT_FALSE(policy.shouldPreempt(running, candidate, clock));
}
