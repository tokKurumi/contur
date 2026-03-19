/// @file priority_policy.cpp
/// @brief Dynamic priority scheduling policy implementation.

#include "contur/scheduling/priority_policy.h"

#include <algorithm>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"

namespace contur {

    namespace {
        [[nodiscard]] bool betterPriority(const PCB &lhs, const PCB &rhs) noexcept
        {
            if (lhs.priority().effective != rhs.priority().effective)
            {
                return static_cast<std::int8_t>(lhs.priority().effective) <
                       static_cast<std::int8_t>(rhs.priority().effective);
            }
            if (lhs.priority().nice != rhs.priority().nice)
            {
                return lhs.priority().nice < rhs.priority().nice;
            }
            return lhs.id() < rhs.id();
        }
    } // namespace

    std::string_view PriorityPolicy::name() const noexcept
    {
        return "Priority";
    }

    ProcessId PriorityPolicy::selectNext(const std::vector<const PCB *> &readyQueue, const IClock &clock) const
    {
        (void)clock;
        if (readyQueue.empty())
        {
            return INVALID_PID;
        }

        const PCB *selected = *std::min_element(readyQueue.begin(), readyQueue.end(), [](const PCB *a, const PCB *b) {
            return betterPriority(*a, *b);
        });
        return selected->id();
    }

    bool PriorityPolicy::shouldPreempt(const PCB &running, const PCB &candidate, const IClock &clock) const
    {
        (void)clock;
        return betterPriority(candidate, running);
    }

} // namespace contur
