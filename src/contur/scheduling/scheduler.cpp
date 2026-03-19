/// @file scheduler.cpp
/// @brief Scheduler implementation.

#include "contur/scheduling/scheduler.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "contur/core/clock.h"

#include "contur/process/pcb.h"
#include "contur/process/state.h"
#include "contur/scheduling/i_scheduling_policy.h"
#include "contur/scheduling/statistics.h"

namespace contur {

    struct Scheduler::Impl
    {
        std::unique_ptr<ISchedulingPolicy> policy;
        Statistics statistics;
        std::unordered_map<ProcessId, PCB *> processes;
        std::unordered_map<ProcessId, Tick> runStart;
        std::vector<PCB *> ready;
        std::vector<PCB *> blocked;
        PCB *running = nullptr;

        explicit Impl(std::unique_ptr<ISchedulingPolicy> policy)
            : policy(std::move(policy))
            , statistics(0.5)
        {}

        [[nodiscard]] bool isKnown(ProcessId pid) const noexcept
        {
            return processes.find(pid) != processes.end();
        }

        [[nodiscard]] static std::vector<const PCB *> asConst(const std::vector<PCB *> &queue)
        {
            std::vector<const PCB *> out;
            out.reserve(queue.size());
            for (const PCB *pcb : queue)
            {
                out.push_back(pcb);
            }
            return out;
        }

        static void removeFrom(std::vector<PCB *> &queue, ProcessId pid)
        {
            queue.erase(
                std::remove_if(queue.begin(), queue.end(), [pid](const PCB *pcb) { return pcb->id() == pid; }),
                queue.end()
            );
        }

        [[nodiscard]] PCB *findIn(std::vector<PCB *> &queue, ProcessId pid)
        {
            auto it = std::find_if(queue.begin(), queue.end(), [pid](const PCB *pcb) { return pcb->id() == pid; });
            if (it == queue.end())
            {
                return nullptr;
            }
            return *it;
        }

        [[nodiscard]] const PCB *findIn(const std::vector<PCB *> &queue, ProcessId pid) const
        {
            auto it = std::find_if(queue.begin(), queue.end(), [pid](const PCB *pcb) { return pcb->id() == pid; });
            if (it == queue.end())
            {
                return nullptr;
            }
            return *it;
        }

        void recordRunningBurst(ProcessId pid, Tick now)
        {
            auto it = runStart.find(pid);
            if (it == runStart.end())
            {
                return;
            }

            Tick started = it->second;
            if (now >= started)
            {
                statistics.recordBurst(pid, now - started);
            }
            runStart.erase(it);
        }
    };

    Scheduler::Scheduler(std::unique_ptr<ISchedulingPolicy> policy)
        : impl_(std::make_unique<Impl>(std::move(policy)))
    {
        if (!impl_->policy)
        {
            throw std::invalid_argument("Scheduler requires a policy");
        }
    }

    Scheduler::~Scheduler() = default;
    Scheduler::Scheduler(Scheduler &&) noexcept = default;
    Scheduler &Scheduler::operator=(Scheduler &&) noexcept = default;

    Result<void> Scheduler::enqueue(PCB &pcb, Tick currentTick)
    {
        if (pcb.id() == INVALID_PID)
        {
            return Result<void>::error(ErrorCode::InvalidPid);
        }

        if (!impl_->isKnown(pcb.id()))
        {
            impl_->processes.emplace(pcb.id(), &pcb);
        }

        if (impl_->findIn(impl_->ready, pcb.id()) != nullptr)
        {
            return Result<void>::ok();
        }

        if (pcb.state() == ProcessState::Running)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        if (!pcb.setState(ProcessState::Ready, currentTick))
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        if (impl_->statistics.hasPrediction(pcb.id()))
        {
            pcb.timing().estimatedBurst = impl_->statistics.predictedBurst(pcb.id());
        }
        impl_->ready.push_back(&pcb);
        return Result<void>::ok();
    }

    Result<void> Scheduler::dequeue(ProcessId pid)
    {
        if (!impl_->isKnown(pid))
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        if (impl_->running != nullptr && impl_->running->id() == pid)
        {
            impl_->running = nullptr;
        }

        Impl::removeFrom(impl_->ready, pid);
        Impl::removeFrom(impl_->blocked, pid);
        impl_->runStart.erase(pid);
        impl_->statistics.clear(pid);
        impl_->processes.erase(pid);

        return Result<void>::ok();
    }

    Result<ProcessId> Scheduler::selectNext(const IClock &clock)
    {
        if (impl_->ready.empty() && impl_->running != nullptr)
        {
            return Result<ProcessId>::ok(impl_->running->id());
        }

        if (impl_->ready.empty())
        {
            return Result<ProcessId>::error(ErrorCode::NotFound);
        }

        auto readyConst = Impl::asConst(impl_->ready);
        ProcessId candidateId = impl_->policy->selectNext(readyConst, clock);
        if (candidateId == INVALID_PID)
        {
            return Result<ProcessId>::error(ErrorCode::InvalidState);
        }

        PCB *candidate = impl_->findIn(impl_->ready, candidateId);
        if (candidate == nullptr)
        {
            return Result<ProcessId>::error(ErrorCode::InvalidState);
        }

        if (impl_->running != nullptr)
        {
            if (!impl_->policy->shouldPreempt(*impl_->running, *candidate, clock))
            {
                return Result<ProcessId>::ok(impl_->running->id());
            }

            ProcessId runningPid = impl_->running->id();
            impl_->recordRunningBurst(runningPid, clock.now());
            if (!impl_->running->setState(ProcessState::Ready, clock.now()))
            {
                return Result<ProcessId>::error(ErrorCode::InvalidState);
            }
            impl_->ready.push_back(impl_->running);
            impl_->running = nullptr;
        }

        Impl::removeFrom(impl_->ready, candidateId);

        if (!candidate->setState(ProcessState::Running, clock.now()))
        {
            return Result<ProcessId>::error(ErrorCode::InvalidState);
        }

        impl_->running = candidate;
        impl_->runStart[candidate->id()] = clock.now();
        return Result<ProcessId>::ok(candidate->id());
    }

    Result<void> Scheduler::blockRunning(Tick currentTick)
    {
        if (impl_->running == nullptr)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        ProcessId pid = impl_->running->id();
        impl_->recordRunningBurst(pid, currentTick);

        if (!impl_->running->setState(ProcessState::Blocked, currentTick))
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        impl_->blocked.push_back(impl_->running);
        impl_->running = nullptr;
        return Result<void>::ok();
    }

    Result<void> Scheduler::unblock(ProcessId pid, Tick currentTick)
    {
        PCB *process = impl_->findIn(impl_->blocked, pid);
        if (process == nullptr)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        if (!process->setState(ProcessState::Ready, currentTick))
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        Impl::removeFrom(impl_->blocked, pid);
        impl_->ready.push_back(process);
        return Result<void>::ok();
    }

    Result<void> Scheduler::terminate(ProcessId pid, Tick currentTick)
    {
        if (!impl_->isKnown(pid))
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        PCB *process = nullptr;
        if (impl_->running != nullptr && impl_->running->id() == pid)
        {
            process = impl_->running;
            impl_->recordRunningBurst(pid, currentTick);
            impl_->running = nullptr;
        }
        else
        {
            process = impl_->processes[pid];
            Impl::removeFrom(impl_->ready, pid);
            Impl::removeFrom(impl_->blocked, pid);
        }

        if (process != nullptr)
        {
            if (!process->setState(ProcessState::Terminated, currentTick))
            {
                return Result<void>::error(ErrorCode::InvalidState);
            }
        }

        impl_->runStart.erase(pid);
        impl_->statistics.clear(pid);
        impl_->processes.erase(pid);

        return Result<void>::ok();
    }

    std::vector<ProcessId> Scheduler::getQueueSnapshot() const
    {
        std::vector<ProcessId> out;
        out.reserve(impl_->ready.size());
        for (const PCB *pcb : impl_->ready)
        {
            out.push_back(pcb->id());
        }
        return out;
    }

    std::vector<ProcessId> Scheduler::getBlockedSnapshot() const
    {
        std::vector<ProcessId> out;
        out.reserve(impl_->blocked.size());
        for (const PCB *pcb : impl_->blocked)
        {
            out.push_back(pcb->id());
        }
        return out;
    }

    ProcessId Scheduler::runningProcess() const noexcept
    {
        if (impl_->running == nullptr)
        {
            return INVALID_PID;
        }
        return impl_->running->id();
    }

    Result<void> Scheduler::setPolicy(std::unique_ptr<ISchedulingPolicy> policy)
    {
        if (!policy)
        {
            return Result<void>::error(ErrorCode::InvalidArgument);
        }

        impl_->policy = std::move(policy);
        return Result<void>::ok();
    }

    std::string_view Scheduler::policyName() const noexcept
    {
        return impl_->policy->name();
    }

} // namespace contur
