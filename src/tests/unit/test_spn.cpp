/// @file test_spn.cpp
/// @brief Unit tests for SPN scheduling policy.

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/scheduling/spn_policy.h"

using namespace contur;

TEST(SpnPolicyTest, SelectsSmallestEstimatedBurst)
{
    SimulationClock clock;
    SpnPolicy policy;

    PCB p1(1, "p1");
    PCB p2(2, "p2");
    PCB p3(3, "p3");

    p1.timing().estimatedBurst = 10;
    p2.timing().estimatedBurst = 3;
    p3.timing().estimatedBurst = 7;

    std::vector<SchedulingProcessSnapshot> ready = {
        {.pid = p1.id(), .estimatedBurst = p1.timing().estimatedBurst},
        {.pid = p2.id(), .estimatedBurst = p2.timing().estimatedBurst},
        {.pid = p3.id(), .estimatedBurst = p3.timing().estimatedBurst},
    };
    EXPECT_EQ(policy.selectNext(ready, clock), 2u);
}

TEST(SpnPolicyTest, ShouldNotPreempt)
{
    SimulationClock clock;
    SpnPolicy policy;
    SchedulingProcessSnapshot running{.pid = 1};
    SchedulingProcessSnapshot candidate{.pid = 2};
    EXPECT_FALSE(policy.shouldPreempt(running, candidate, clock));
}
