/// @file fcfs_policy.cpp
/// @brief FCFS scheduling policy implementation.

#include "contur/scheduling/fcfs_policy.h"

#include <algorithm>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"

namespace contur {

    std::string_view FcfsPolicy::name() const noexcept
    {
        return "FCFS";
    }

    ProcessId FcfsPolicy::selectNext(const std::vector<const PCB *> &readyQueue, const IClock &clock) const
    {
        (void)clock;
        if (readyQueue.empty())
        {
            return INVALID_PID;
        }

        const PCB *selected = *std::min_element(readyQueue.begin(), readyQueue.end(), [](const PCB *a, const PCB *b) {
            if (a->timing().arrivalTime != b->timing().arrivalTime)
            {
                return a->timing().arrivalTime < b->timing().arrivalTime;
            }
            return a->id() < b->id();
        });

        return selected->id();
    }

    bool FcfsPolicy::shouldPreempt(const PCB &running, const PCB &candidate, const IClock &clock) const
    {
        (void)running;
        (void)candidate;
        (void)clock;
        return false;
    }

} // namespace contur
