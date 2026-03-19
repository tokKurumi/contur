/// @file mp_dispatcher.h
/// @brief Multiprocessor dispatcher orchestrating multiple child dispatchers.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "contur/dispatch/i_dispatcher.h"

namespace contur {

    class MPDispatcher final : public IDispatcher
    {
        public:
        explicit MPDispatcher(std::vector<std::reference_wrapper<IDispatcher>> dispatchers);
        ~MPDispatcher() override;

        MPDispatcher(const MPDispatcher &) = delete;
        MPDispatcher &operator=(const MPDispatcher &) = delete;
        MPDispatcher(MPDispatcher &&) noexcept;
        MPDispatcher &operator=(MPDispatcher &&) noexcept;

        [[nodiscard]] Result<void> createProcess(std::unique_ptr<ProcessImage> process, Tick currentTick) override;
        [[nodiscard]] Result<void> dispatch(std::size_t tickBudget) override;
        void tick() override;
        [[nodiscard]] std::size_t processCount() const noexcept override;
        [[nodiscard]] bool hasProcess(ProcessId pid) const noexcept override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
