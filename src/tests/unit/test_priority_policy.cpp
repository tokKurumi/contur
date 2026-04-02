/// @file test_priority_policy.cpp
/// @brief Unit tests for Priority scheduling policy.

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/scheduling/priority_policy.h"

using namespace contur;

TEST(PriorityPolicyTest, SelectsHighestEffectivePriority)
{
    SimulationClock clock;
    PriorityPolicy policy;

    PCB low(1, "low", Priority(PriorityLevel::Low));
    PCB high(2, "high", Priority(PriorityLevel::High));
    PCB normal(3, "normal", Priority(PriorityLevel::Normal));

    std::vector<SchedulingProcessSnapshot> ready = {
        {.pid = low.id(), .effectivePriority = low.priority().effective, .nice = low.priority().nice},
        {.pid = high.id(), .effectivePriority = high.priority().effective, .nice = high.priority().nice},
        {.pid = normal.id(), .effectivePriority = normal.priority().effective, .nice = normal.priority().nice},
    };
    EXPECT_EQ(policy.selectNext(ready, clock), 2u);
}

TEST(PriorityPolicyTest, PreemptsWhenCandidateHigherPriority)
{
    SimulationClock clock;
    PriorityPolicy policy;

    SchedulingProcessSnapshot running{.pid = 1, .effectivePriority = PriorityLevel::Normal};
    SchedulingProcessSnapshot candidate{.pid = 2, .effectivePriority = PriorityLevel::High};

    EXPECT_TRUE(policy.shouldPreempt(running, candidate, clock));
}
