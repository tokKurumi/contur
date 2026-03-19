/// @file i_scheduler.h
/// @brief IScheduler interface.

#pragma once

#include <memory>
#include <vector>

#include "contur/core/error.h"
#include "contur/core/types.h"

namespace contur {

    class IClock;
    class PCB;
    class ISchedulingPolicy;

    /// @brief Scheduler abstraction managing process state queues.
    class IScheduler
    {
        public:
        virtual ~IScheduler() = default;

        /// @brief Enqueues a process into the ready queue.
        [[nodiscard]] virtual Result<void> enqueue(PCB &pcb, Tick currentTick) = 0;

        /// @brief Removes a process from scheduler ownership/queues.
        [[nodiscard]] virtual Result<void> dequeue(ProcessId pid) = 0;

        /// @brief Selects the process to run next.
        [[nodiscard]] virtual Result<ProcessId> selectNext(const IClock &clock) = 0;

        /// @brief Moves current running process to blocked queue.
        [[nodiscard]] virtual Result<void> blockRunning(Tick currentTick) = 0;

        /// @brief Moves a blocked process back to ready queue.
        [[nodiscard]] virtual Result<void> unblock(ProcessId pid, Tick currentTick) = 0;

        /// @brief Marks process as terminated and removes it from active queues.
        [[nodiscard]] virtual Result<void> terminate(ProcessId pid, Tick currentTick) = 0;

        /// @brief Returns ready queue PID snapshot.
        [[nodiscard]] virtual std::vector<ProcessId> getQueueSnapshot() const = 0;

        /// @brief Returns blocked queue PID snapshot.
        [[nodiscard]] virtual std::vector<ProcessId> getBlockedSnapshot() const = 0;

        /// @brief Returns currently running process, or INVALID_PID.
        [[nodiscard]] virtual ProcessId runningProcess() const noexcept = 0;

        /// @brief Replaces scheduling policy at runtime.
        [[nodiscard]] virtual Result<void> setPolicy(std::unique_ptr<ISchedulingPolicy> policy) = 0;
    };

} // namespace contur
