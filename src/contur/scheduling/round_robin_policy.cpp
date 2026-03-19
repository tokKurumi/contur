/// @file round_robin_policy.cpp
/// @brief Round Robin scheduling policy implementation.

#include "contur/scheduling/round_robin_policy.h"

#include <algorithm>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"

namespace contur {

    RoundRobinPolicy::RoundRobinPolicy(std::size_t timeSlice)
        : timeSlice_(timeSlice == 0 ? 1 : timeSlice)
    {}

    std::string_view RoundRobinPolicy::name() const noexcept
    {
        return "RoundRobin";
    }

    ProcessId RoundRobinPolicy::selectNext(const std::vector<const PCB *> &readyQueue, const IClock &clock) const
    {
        (void)clock;
        if (readyQueue.empty())
        {
            return INVALID_PID;
        }

        const PCB *selected = *std::min_element(readyQueue.begin(), readyQueue.end(), [](const PCB *a, const PCB *b) {
            if (a->timing().lastStateChange != b->timing().lastStateChange)
            {
                return a->timing().lastStateChange < b->timing().lastStateChange;
            }
            return a->id() < b->id();
        });
        return selected->id();
    }

    bool RoundRobinPolicy::shouldPreempt(const PCB &running, const PCB &candidate, const IClock &clock) const
    {
        (void)candidate;
        Tick elapsed = clock.now() - running.timing().lastStateChange;
        return elapsed >= timeSlice_;
    }

    std::size_t RoundRobinPolicy::timeSlice() const noexcept
    {
        return timeSlice_;
    }

} // namespace contur
