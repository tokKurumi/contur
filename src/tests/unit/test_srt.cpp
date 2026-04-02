/// @file test_srt.cpp
/// @brief Unit tests for SRT scheduling policy.

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/scheduling/srt_policy.h"

using namespace contur;

TEST(SrtPolicyTest, SelectsSmallestRemainingBurst)
{
    SimulationClock clock;
    SrtPolicy policy;

    PCB p1(1, "p1");
    PCB p2(2, "p2");
    PCB p3(3, "p3");

    p1.timing().remainingBurst = 8;
    p2.timing().remainingBurst = 2;
    p3.timing().remainingBurst = 5;

    std::vector<SchedulingProcessSnapshot> ready = {
        {.pid = p1.id(), .remainingBurst = p1.timing().remainingBurst},
        {.pid = p2.id(), .remainingBurst = p2.timing().remainingBurst},
        {.pid = p3.id(), .remainingBurst = p3.timing().remainingBurst},
    };
    EXPECT_EQ(policy.selectNext(ready, clock), 2u);
}

TEST(SrtPolicyTest, PreemptsWhenCandidateHasSmallerRemaining)
{
    SimulationClock clock;
    SrtPolicy policy;

    SchedulingProcessSnapshot running{.pid = 1, .remainingBurst = 6};
    SchedulingProcessSnapshot candidate{.pid = 2, .remainingBurst = 2};

    EXPECT_TRUE(policy.shouldPreempt(running, candidate, clock));
}
