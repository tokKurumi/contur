/// @file test_policy_contracts.cpp
/// @brief Contract tests for scheduling policy snapshot isolation.

#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/scheduling/fcfs_policy.h"
#include "contur/scheduling/i_scheduling_policy.h"
#include "contur/scheduling/spn_policy.h"

using namespace contur;

TEST(PolicyContractTest, SnapshotTypeIsValueBased)
{
    static_assert(!std::is_reference_v<SchedulingProcessSnapshot>);
    static_assert(std::is_trivially_copyable_v<SchedulingProcessSnapshot>);
    SUCCEED();
}

TEST(PolicyContractTest, SelectionUsesSnapshotAndNotLivePcbState)
{
    SimulationClock clock;
    SpnPolicy policy;

    PCB p1(1, "p1");
    PCB p2(2, "p2");

    p1.timing().estimatedBurst = 1;
    p2.timing().estimatedBurst = 10;

    std::vector<SchedulingProcessSnapshot> ready = {
        {.pid = p1.id(), .estimatedBurst = p1.timing().estimatedBurst},
        {.pid = p2.id(), .estimatedBurst = p2.timing().estimatedBurst},
    };

    // Mutate live PCB state after snapshots are formed.
    p1.timing().estimatedBurst = 100;

    EXPECT_EQ(policy.selectNext(ready, clock), 1u);
}

TEST(PolicyContractTest, PreemptionDecisionDoesNotMutateSnapshots)
{
    SimulationClock clock;
    FcfsPolicy policy;

    SchedulingProcessSnapshot running{.pid = 1, .arrivalTime = 1};
    SchedulingProcessSnapshot candidate{.pid = 2, .arrivalTime = 2};

    SchedulingProcessSnapshot runningBefore = running;
    SchedulingProcessSnapshot candidateBefore = candidate;

    EXPECT_FALSE(policy.shouldPreempt(running, candidate, clock));
    EXPECT_EQ(running.pid, runningBefore.pid);
    EXPECT_EQ(running.arrivalTime, runningBefore.arrivalTime);
    EXPECT_EQ(candidate.pid, candidateBefore.pid);
    EXPECT_EQ(candidate.arrivalTime, candidateBefore.arrivalTime);
}
