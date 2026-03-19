/// @file critical_section.h
/// @brief CriticalSection wrapper composed over an ISyncPrimitive.

#pragma once

#include <memory>

#include "contur/sync/i_sync_primitive.h"

namespace contur {

    class CriticalSection final : public ISyncPrimitive
    {
        public:
        explicit CriticalSection(std::unique_ptr<ISyncPrimitive> primitive = nullptr);
        ~CriticalSection() override;

        CriticalSection(const CriticalSection &) = delete;
        CriticalSection &operator=(const CriticalSection &) = delete;
        CriticalSection(CriticalSection &&) noexcept;
        CriticalSection &operator=(CriticalSection &&) noexcept;

        [[nodiscard]] Result<void> acquire(ProcessId pid) override;
        [[nodiscard]] Result<void> release(ProcessId pid) override;
        [[nodiscard]] Result<void> tryAcquire(ProcessId pid) override;
        [[nodiscard]] std::string_view name() const noexcept override;

        [[nodiscard]] Result<void> enter(ProcessId pid);
        [[nodiscard]] Result<void> leave(ProcessId pid);
        [[nodiscard]] Result<void> tryEnter(ProcessId pid);

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
