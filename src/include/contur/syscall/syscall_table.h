/// @file syscall_table.h
/// @brief SyscallTable registry and dispatch for system calls.

#pragma once

#include <functional>
#include <memory>
#include <span>

#include "contur/core/error.h"
#include "contur/core/types.h"

#include "contur/syscall/syscall_handler.h"
#include "contur/syscall/syscall_ids.h"

namespace contur {

    class ProcessImage;

    /// @brief Dispatch table mapping SyscallId to handler functions.
    class SyscallTable
    {
        public:
        /// @brief Function signature used for syscall handlers.
        using HandlerFn = std::function<Result<RegisterValue>(std::span<const RegisterValue>, ProcessImage &)>;

        SyscallTable();
        ~SyscallTable();

        SyscallTable(const SyscallTable &) = delete;
        SyscallTable &operator=(const SyscallTable &) = delete;
        SyscallTable(SyscallTable &&) noexcept;
        SyscallTable &operator=(SyscallTable &&) noexcept;

        /// @brief Registers/replaces a function handler for syscall id.
        /// @return Ok on success, InvalidArgument for empty function.
        [[nodiscard]] Result<void> registerHandler(SyscallId id, HandlerFn handler);

        /// @brief Registers/replaces an interface-based handler for syscall id.
        ///
        /// The table stores a lightweight wrapper that forwards to @p handler.
        [[nodiscard]] Result<void> registerHandler(SyscallId id, ISyscallHandler &handler);

        /// @brief Unregisters a handler for syscall id.
        /// @return Ok on success or NotFound.
        [[nodiscard]] Result<void> unregisterHandler(SyscallId id);

        /// @brief Dispatches syscall to registered handler.
        /// @return Handler result or NotFound when no handler is registered.
        [[nodiscard]] Result<RegisterValue>
        dispatch(SyscallId id, std::span<const RegisterValue> args, ProcessImage &caller) const;

        /// @brief Returns true when id has a registered handler.
        [[nodiscard]] bool hasHandler(SyscallId id) const noexcept;

        /// @brief Number of registered handlers.
        [[nodiscard]] std::size_t handlerCount() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
