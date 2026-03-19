/// @file i_sync_primitive.h
/// @brief ISyncPrimitive interface for synchronization objects.

#pragma once

#include <string_view>

#include "contur/core/error.h"
#include "contur/core/types.h"

namespace contur {

    class ISyncPrimitive
    {
        public:
        virtual ~ISyncPrimitive() = default;

        /// @brief Attempts to acquire the primitive for a process.
        [[nodiscard]] virtual Result<void> acquire(ProcessId pid) = 0;

        /// @brief Releases the primitive for a process.
        [[nodiscard]] virtual Result<void> release(ProcessId pid) = 0;

        /// @brief Attempts non-blocking acquire.
        [[nodiscard]] virtual Result<void> tryAcquire(ProcessId pid) = 0;

        /// @brief Human-readable primitive name.
        [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    };

} // namespace contur
