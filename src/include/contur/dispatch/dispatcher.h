/// @file dispatcher.h
/// @brief Dispatcher implementation.

#pragma once

#include <memory>

#include "contur/dispatch/i_dispatcher.h"

namespace contur {

    class IClock;
    class IExecutionEngine;
    class IScheduler;
    class IVirtualMemory;

    class Dispatcher final : public IDispatcher
    {
        public:
        Dispatcher(IScheduler &scheduler, IExecutionEngine &engine, IVirtualMemory &virtualMemory, IClock &clock);
        ~Dispatcher() override;

        Dispatcher(const Dispatcher &) = delete;
        Dispatcher &operator=(const Dispatcher &) = delete;
        Dispatcher(Dispatcher &&) noexcept;
        Dispatcher &operator=(Dispatcher &&) noexcept;

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
