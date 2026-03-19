/// @file semaphore.h
/// @brief Counting semaphore synchronization primitive.

#pragma once

#include <memory>

#include "contur/sync/i_sync_primitive.h"

namespace contur {

    class Semaphore final : public ISyncPrimitive
    {
        public:
        explicit Semaphore(std::size_t initialCount = 1, std::size_t maxCount = 1);
        ~Semaphore() override;

        Semaphore(const Semaphore &) = delete;
        Semaphore &operator=(const Semaphore &) = delete;
        Semaphore(Semaphore &&) noexcept;
        Semaphore &operator=(Semaphore &&) noexcept;

        [[nodiscard]] Result<void> acquire(ProcessId pid) override;
        [[nodiscard]] Result<void> release(ProcessId pid) override;
        [[nodiscard]] Result<void> tryAcquire(ProcessId pid) override;
        [[nodiscard]] std::string_view name() const noexcept override;

        [[nodiscard]] std::size_t count() const noexcept;
        [[nodiscard]] std::size_t maxCount() const noexcept;
        [[nodiscard]] std::size_t waitingCount() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
