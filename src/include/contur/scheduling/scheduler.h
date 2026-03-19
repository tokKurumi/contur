/// @file scheduler.h
/// @brief Scheduler implementation hosting a pluggable policy.

#pragma once

#include <memory>

#include "contur/scheduling/i_scheduler.h"

namespace contur {

    class Scheduler final : public IScheduler
    {
        public:
        explicit Scheduler(std::unique_ptr<ISchedulingPolicy> policy);
        ~Scheduler() override;

        Scheduler(const Scheduler &) = delete;
        Scheduler &operator=(const Scheduler &) = delete;
        Scheduler(Scheduler &&) noexcept;
        Scheduler &operator=(Scheduler &&) noexcept;

        [[nodiscard]] Result<void> enqueue(PCB &pcb, Tick currentTick) override;
        [[nodiscard]] Result<void> dequeue(ProcessId pid) override;
        [[nodiscard]] Result<ProcessId> selectNext(const IClock &clock) override;
        [[nodiscard]] Result<void> blockRunning(Tick currentTick) override;
        [[nodiscard]] Result<void> unblock(ProcessId pid, Tick currentTick) override;
        [[nodiscard]] Result<void> terminate(ProcessId pid, Tick currentTick) override;
        [[nodiscard]] std::vector<ProcessId> getQueueSnapshot() const override;
        [[nodiscard]] std::vector<ProcessId> getBlockedSnapshot() const override;
        [[nodiscard]] ProcessId runningProcess() const noexcept override;
        [[nodiscard]] Result<void> setPolicy(std::unique_ptr<ISchedulingPolicy> policy) override;

        /// @brief Returns active policy name.
        [[nodiscard]] std::string_view policyName() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
