/// @file deadlock_detector.cpp
/// @brief DeadlockDetector implementation.

#include "contur/sync/deadlock_detector.h"

#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace contur {

    struct DeadlockDetector::Impl
    {
        std::unordered_map<ResourceId, std::unordered_set<ProcessId>> resourceOwners;
        std::unordered_map<ProcessId, std::unordered_set<ProcessId>> waitFor;

        void clearOutgoing(ProcessId pid)
        {
            waitFor.erase(pid);
        }

        [[nodiscard]] bool hasCycle() const
        {
            std::unordered_set<ProcessId> visited;
            std::unordered_set<ProcessId> onStack;

            std::function<bool(ProcessId)> dfs = [&](ProcessId node) {
                if (onStack.find(node) != onStack.end())
                {
                    return true;
                }
                if (visited.find(node) != visited.end())
                {
                    return false;
                }

                visited.insert(node);
                onStack.insert(node);

                auto it = waitFor.find(node);
                if (it != waitFor.end())
                {
                    for (ProcessId next : it->second)
                    {
                        if (dfs(next))
                        {
                            return true;
                        }
                    }
                }

                onStack.erase(node);
                return false;
            };

            for (const auto &[pid, _] : waitFor)
            {
                if (dfs(pid))
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] std::vector<ProcessId> cycleNodes() const
        {
            std::set<ProcessId> cycleSet;
            std::unordered_set<ProcessId> visited;
            std::vector<ProcessId> stack;

            std::function<void(ProcessId)> dfs = [&](ProcessId node) {
                visited.insert(node);
                stack.push_back(node);

                auto it = waitFor.find(node);
                if (it != waitFor.end())
                {
                    for (ProcessId next : it->second)
                    {
                        auto pos = std::find(stack.begin(), stack.end(), next);
                        if (pos != stack.end())
                        {
                            for (auto cyc = pos; cyc != stack.end(); ++cyc)
                            {
                                cycleSet.insert(*cyc);
                            }
                        }
                        else if (visited.find(next) == visited.end())
                        {
                            dfs(next);
                        }
                    }
                }

                stack.pop_back();
            };

            for (const auto &[pid, _] : waitFor)
            {
                if (visited.find(pid) == visited.end())
                {
                    dfs(pid);
                }
            }

            return std::vector<ProcessId>(cycleSet.begin(), cycleSet.end());
        }
    };

    DeadlockDetector::DeadlockDetector()
        : impl_(std::make_unique<Impl>())
    {}

    DeadlockDetector::~DeadlockDetector() = default;
    DeadlockDetector::DeadlockDetector(DeadlockDetector &&) noexcept = default;
    DeadlockDetector &DeadlockDetector::operator=(DeadlockDetector &&) noexcept = default;

    void DeadlockDetector::onAcquire(ProcessId pid, ResourceId resource)
    {
        if (pid == INVALID_PID)
        {
            return;
        }

        impl_->resourceOwners[resource].insert(pid);
        impl_->clearOutgoing(pid);
    }

    void DeadlockDetector::onRelease(ProcessId pid, ResourceId resource)
    {
        auto ownerIt = impl_->resourceOwners.find(resource);
        if (ownerIt == impl_->resourceOwners.end())
        {
            return;
        }

        ownerIt->second.erase(pid);
        if (ownerIt->second.empty())
        {
            impl_->resourceOwners.erase(ownerIt);
        }
    }

    void DeadlockDetector::onWait(ProcessId pid, ResourceId resource)
    {
        if (pid == INVALID_PID)
        {
            return;
        }

        impl_->clearOutgoing(pid);

        auto ownerIt = impl_->resourceOwners.find(resource);
        if (ownerIt == impl_->resourceOwners.end())
        {
            return;
        }

        for (ProcessId owner : ownerIt->second)
        {
            if (owner != pid)
            {
                impl_->waitFor[pid].insert(owner);
            }
        }
    }

    bool DeadlockDetector::hasDeadlock() const
    {
        return impl_->hasCycle();
    }

    std::vector<ProcessId> DeadlockDetector::getDeadlockedProcesses() const
    {
        return impl_->cycleNodes();
    }

    bool DeadlockDetector::isSafeState(
        const std::vector<ResourceAllocation> &current,
        const std::vector<ResourceAllocation> &maximum,
        const std::vector<std::uint32_t> &available
    ) const
    {
        if (current.size() != maximum.size())
        {
            return false;
        }
        if (current.empty())
        {
            return true;
        }

        const std::size_t processCount = current.size();
        const std::size_t resourceCount = available.size();
        if (resourceCount == 0)
        {
            return true;
        }

        std::unordered_map<ProcessId, std::size_t> indexByPid;
        indexByPid.reserve(processCount);

        for (std::size_t i = 0; i < processCount; ++i)
        {
            if (current[i].pid == INVALID_PID || maximum[i].pid == INVALID_PID)
            {
                return false;
            }
            if (current[i].pid != maximum[i].pid)
            {
                return false;
            }
            if (current[i].resources.size() != resourceCount || maximum[i].resources.size() != resourceCount)
            {
                return false;
            }
            if (indexByPid.find(current[i].pid) != indexByPid.end())
            {
                return false;
            }
            indexByPid[current[i].pid] = i;
        }

        for (std::size_t i = 0; i < processCount; ++i)
        {
            for (std::size_t r = 0; r < resourceCount; ++r)
            {
                if (current[i].resources[r] > maximum[i].resources[r])
                {
                    return false;
                }
            }
        }

        std::vector<std::uint32_t> work = available;

        std::vector<bool> finished(processCount, false);
        std::size_t finishCount = 0;

        while (true)
        {
            bool progressed = false;
            for (std::size_t i = 0; i < processCount; ++i)
            {
                if (finished[i])
                {
                    continue;
                }

                bool canFinish = true;
                for (std::size_t r = 0; r < resourceCount; ++r)
                {
                    std::uint32_t need = maximum[i].resources[r] - current[i].resources[r];
                    if (need > work[r])
                    {
                        canFinish = false;
                        break;
                    }
                }

                if (canFinish)
                {
                    for (std::size_t r = 0; r < resourceCount; ++r)
                    {
                        work[r] += current[i].resources[r];
                    }
                    finished[i] = true;
                    ++finishCount;
                    progressed = true;
                }
            }

            if (!progressed)
            {
                break;
            }
        }

        return finishCount == processCount;
    }

} // namespace contur
