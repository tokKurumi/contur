/// @file mutex.h
/// @brief Mutex synchronization primitive.

#pragma once

#include <memory>
#include <optional>

#include "contur/sync/i_sync_primitive.h"

namespace contur {

    class Mutex final : public ISyncPrimitive
    {
        public:
        Mutex();
        ~Mutex() override;

        Mutex(const Mutex &) = delete;
        Mutex &operator=(const Mutex &) = delete;
        Mutex(Mutex &&) noexcept;
        Mutex &operator=(Mutex &&) noexcept;

        [[nodiscard]] Result<void> acquire(ProcessId pid) override;
        [[nodiscard]] Result<void> release(ProcessId pid) override;
        [[nodiscard]] Result<void> tryAcquire(ProcessId pid) override;
        [[nodiscard]] std::string_view name() const noexcept override;

        [[nodiscard]] bool isLocked() const noexcept;
        [[nodiscard]] std::optional<ProcessId> owner() const noexcept;
        [[nodiscard]] std::size_t recursionDepth() const noexcept;
        [[nodiscard]] std::size_t waitingCount() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
