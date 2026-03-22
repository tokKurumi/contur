/// @file syscall_handler.h
/// @brief ISyscallHandler interface.

#pragma once

#include <span>

#include "contur/core/error.h"
#include "contur/core/types.h"

#include "contur/syscall/syscall_ids.h"

namespace contur {

    class ProcessImage;

    /// @brief Interface for syscall handling.
    ///
    /// Implementations execute kernel-side logic for a syscall and return
    /// register-compatible results.
    class ISyscallHandler
    {
        public:
        virtual ~ISyscallHandler() = default;

        /// @brief Handles a syscall request.
        /// @param id Syscall identifier.
        /// @param args Syscall arguments from user context.
        /// @param caller Calling process image.
        /// @return RegisterValue result or an error code.
        [[nodiscard]] virtual Result<RegisterValue>
        handle(SyscallId id, std::span<const RegisterValue> args, ProcessImage &caller) = 0;
    };

} // namespace contur
