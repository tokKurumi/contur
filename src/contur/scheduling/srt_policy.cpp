/// @file srt_policy.cpp
/// @brief SRT scheduling policy implementation.

#include "contur/scheduling/srt_policy.h"

#include <algorithm>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"

namespace contur {

    namespace {
        [[nodiscard]] Tick remaining(const PCB &pcb) noexcept
        {
            if (pcb.timing().remainingBurst > 0)
            {
                return pcb.timing().remainingBurst;
            }
            if (pcb.timing().estimatedBurst > 0)
            {
                return pcb.timing().estimatedBurst;
            }
            return 1;
        }
    } // namespace

    std::string_view SrtPolicy::name() const noexcept
    {
        return "SRT";
    }

    ProcessId SrtPolicy::selectNext(const std::vector<const PCB *> &readyQueue, const IClock &clock) const
    {
        (void)clock;
        if (readyQueue.empty())
        {
            return INVALID_PID;
        }

        const PCB *selected = *std::min_element(readyQueue.begin(), readyQueue.end(), [](const PCB *a, const PCB *b) {
            Tick lhs = remaining(*a);
            Tick rhs = remaining(*b);
            if (lhs != rhs)
            {
                return lhs < rhs;
            }
            return a->id() < b->id();
        });
        return selected->id();
    }

    bool SrtPolicy::shouldPreempt(const PCB &running, const PCB &candidate, const IClock &clock) const
    {
        (void)clock;
        return remaining(candidate) < remaining(running);
    }

} // namespace contur
