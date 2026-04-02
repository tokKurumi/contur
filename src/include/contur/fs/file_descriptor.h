/// @file file_descriptor.h
/// @brief File descriptor types and descriptor-table abstraction.

#pragma once

#include <cstdint>
#include <memory>

#include "contur/core/error.h"

#include "contur/fs/inode.h"

namespace contur {

    /// @brief Descriptor handle returned by open().
    struct FileDescriptor
    {
        /// @brief Underlying descriptor value.
        std::int32_t value = -1;

        /// @brief Returns true when descriptor is valid.
        /// @return True if value >= 0.
        [[nodiscard]] constexpr bool valid() const noexcept
        {
            return value >= 0;
        }

        /// @brief Equality comparison for descriptor values.
        /// @param lhs Left descriptor.
        /// @param rhs Right descriptor.
        /// @return True when both descriptors hold the same value.
        friend constexpr bool operator==(FileDescriptor lhs, FileDescriptor rhs) noexcept
        {
            return lhs.value == rhs.value;
        }
    };

    /// @brief Open mode bit flags.
    enum class OpenMode : std::uint8_t
    {
        None = 0,
        Read = 1U << 0,
        Write = 1U << 1,
        Create = 1U << 2,
        Truncate = 1U << 3,
        Append = 1U << 4,
    };

    /// @brief Returns bitwise OR combination of open-mode flags.
    [[nodiscard]] constexpr OpenMode operator|(OpenMode lhs, OpenMode rhs) noexcept
    {
        return static_cast<OpenMode>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
    }

    /// @brief Returns bitwise AND combination of open-mode flags.
    [[nodiscard]] constexpr OpenMode operator&(OpenMode lhs, OpenMode rhs) noexcept
    {
        return static_cast<OpenMode>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
    }

    /// @brief Returns whether the provided mode set contains a flag.
    /// @param value Composite open mode.
    /// @param flag Single flag to test.
    /// @return True when flag exists in value.
    [[nodiscard]] constexpr bool hasOpenMode(OpenMode value, OpenMode flag) noexcept
    {
        return static_cast<std::uint8_t>(value & flag) != 0;
    }

    /// @brief Runtime state for one open descriptor.
    struct OpenFileState
    {
        /// @brief Target inode associated with descriptor.
        InodeId inodeId = INVALID_INODE_ID;

        /// @brief Current byte offset.
        std::size_t offset = 0;

        /// @brief Open mode flags for this descriptor.
        OpenMode mode = OpenMode::None;
    };

    /// @brief Descriptor table mapping integer handles to open-file state.
    class FileDescriptorTable final
    {
        public:
        /// @brief Constructs an empty file descriptor table.
        FileDescriptorTable();

        /// @brief Destroys descriptor table.
        ~FileDescriptorTable();

        /// @brief Copy construction is disabled.
        FileDescriptorTable(const FileDescriptorTable &) = delete;

        /// @brief Copy assignment is disabled.
        FileDescriptorTable &operator=(const FileDescriptorTable &) = delete;
        /// @brief Move-constructs descriptor table state.
        FileDescriptorTable(FileDescriptorTable &&) noexcept;

        /// @brief Move-assigns descriptor table state.
        FileDescriptorTable &operator=(FileDescriptorTable &&) noexcept;

        /// @brief Creates a new descriptor entry.
        /// @param state Initial open-file state to store.
        /// @return Newly allocated descriptor handle.
        [[nodiscard]] Result<FileDescriptor> open(OpenFileState state);

        /// @brief Removes a descriptor entry.
        /// @param fd Descriptor to close.
        /// @return Ok on success; NotFound if descriptor does not exist.
        [[nodiscard]] Result<void> close(FileDescriptor fd);

        /// @brief Returns state for descriptor.
        /// @param fd Descriptor to query.
        /// @return Open-file state or NotFound.
        [[nodiscard]] Result<OpenFileState> get(FileDescriptor fd) const;

        /// @brief Replaces state for descriptor.
        /// @param fd Descriptor to update.
        /// @param state New state value.
        /// @return Ok on success; NotFound if descriptor does not exist.
        [[nodiscard]] Result<void> set(FileDescriptor fd, OpenFileState state);

        /// @brief Returns whether descriptor exists.
        /// @param fd Descriptor to test.
        /// @return True when descriptor is open.
        [[nodiscard]] bool contains(FileDescriptor fd) const noexcept;

        /// @brief Count of currently open descriptors.
        /// @return Number of open entries in table.
        [[nodiscard]] std::size_t openCount() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
