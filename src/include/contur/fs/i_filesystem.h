/// @file i_filesystem.h
/// @brief IFileSystem interface for file-system operations.

#pragma once

#include <span>
#include <string>
#include <vector>

#include "contur/core/error.h"

#include "contur/fs/directory_entry.h"
#include "contur/fs/file_descriptor.h"

namespace contur {

    /// @brief Public inode metadata returned by stat().
    struct InodeInfo
    {
        /// @brief Inode identifier.
        InodeId id = INVALID_INODE_ID;

        /// @brief Inode node type.
        InodeType type = InodeType::File;

        /// @brief Logical size in bytes.
        std::size_t size = 0;

        /// @brief Number of allocated data blocks.
        std::size_t blockCount = 0;

        /// @brief Creation timestamp in ticks.
        Tick createdAt = 0;

        /// @brief Last-modification timestamp in ticks.
        Tick modifiedAt = 0;
    };

    /// @brief Abstract file-system interface.
    class IFileSystem
    {
        public:
        virtual ~IFileSystem() = default;

        /// @brief Opens a file path with the requested mode.
        /// @param path Absolute file path.
        /// @param mode Open mode flags.
        /// @return File descriptor on success; error otherwise.
        [[nodiscard]] virtual Result<FileDescriptor> open(const std::string &path, OpenMode mode) = 0;

        /// @brief Reads bytes from an open descriptor.
        /// @param fd Open descriptor.
        /// @param buffer Output buffer.
        /// @return Number of bytes read, or error.
        [[nodiscard]] virtual Result<std::size_t> read(FileDescriptor fd, std::span<std::byte> buffer) = 0;

        /// @brief Writes bytes to an open descriptor.
        /// @param fd Open descriptor.
        /// @param data Data bytes to write.
        /// @return Number of bytes written, or error.
        [[nodiscard]] virtual Result<std::size_t> write(FileDescriptor fd, std::span<const std::byte> data) = 0;

        /// @brief Closes an open descriptor.
        /// @param fd Descriptor to close.
        /// @return Ok on success; NotFound for unknown descriptor.
        [[nodiscard]] virtual Result<void> close(FileDescriptor fd) = 0;

        /// @brief Creates a directory at path.
        /// @param path Absolute directory path.
        /// @return Ok on success; AlreadyExists/NotFound/InvalidArgument otherwise.
        [[nodiscard]] virtual Result<void> mkdir(const std::string &path) = 0;

        /// @brief Removes a file or an empty directory.
        /// @param path Absolute path to remove.
        /// @return Ok on success; NotFound/InvalidState on failure.
        [[nodiscard]] virtual Result<void> remove(const std::string &path) = 0;

        /// @brief Lists immediate child entries in a directory.
        /// @param path Absolute directory path.
        /// @return Vector of directory entries, or error.
        [[nodiscard]] virtual Result<std::vector<DirectoryEntry>> listDir(const std::string &path) const = 0;

        /// @brief Returns metadata for path.
        /// @param path Absolute path of file or directory.
        /// @return Inode info or NotFound.
        [[nodiscard]] virtual Result<InodeInfo> stat(const std::string &path) const = 0;
    };

} // namespace contur
