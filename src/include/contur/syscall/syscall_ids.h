/// @file syscall_ids.h
/// @brief Syscall IDs for the user-kernel boundary.

#pragma once

#include <cstdint>

namespace contur {

    /// @brief Numeric identifiers for system calls.
    enum class SyscallId : std::uint16_t
    {
        /// @brief Terminate current process.
        Exit = 0,
        /// @brief Write bytes to a kernel-managed sink.
        Write = 1,
        /// @brief Read bytes from a kernel-managed source.
        Read = 2,
        /// @brief Create child process.
        Fork = 3,
        /// @brief Replace process image.
        Exec = 4,
        /// @brief Wait for child process.
        Wait = 5,
        /// @brief Open file/device.
        Open = 10,
        /// @brief Close file/device.
        Close = 11,
        /// @brief Create IPC pipe.
        CreatePipe = 20,
        /// @brief Send message to message queue.
        SendMessage = 21,
        /// @brief Receive message from message queue.
        ReceiveMessage = 22,
        /// @brief Create shared-memory region.
        ShmCreate = 23,
        /// @brief Attach to shared-memory region.
        ShmAttach = 24,
        /// @brief Lock mutex.
        MutexLock = 30,
        /// @brief Unlock mutex.
        MutexUnlock = 31,
        /// @brief Wait/decrement semaphore.
        SemWait = 32,
        /// @brief Signal/increment semaphore.
        SemSignal = 33,
        /// @brief Query current process id.
        GetPid = 40,
        /// @brief Query simulation time.
        GetTime = 41,
        /// @brief Voluntarily yield CPU.
        Yield = 42,
    };

} // namespace contur
