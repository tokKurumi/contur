/// @file simple_fs.h
/// @brief Simple in-memory inode-based filesystem implementation.

#pragma once

#include <memory>

#include "contur/fs/i_filesystem.h"

namespace contur {

    /// @brief In-memory educational filesystem with inode + block allocation model.
    ///
    /// SimpleFS keeps all metadata and block data in memory and implements
    /// basic directory traversal, file read/write via descriptors, and stat/list
    /// operations suitable for kernel-simulator demos and tests.
    class SimpleFS final : public IFileSystem
    {
        public:
        /// @brief Constructs filesystem over simulated disk blocks.
        /// @param totalBlocks Number of simulated disk blocks.
        /// @param blockSize Size of each block in bytes.
        explicit SimpleFS(std::size_t totalBlocks = 256, std::size_t blockSize = 512);
        ~SimpleFS() override;

        SimpleFS(const SimpleFS &) = delete;
        SimpleFS &operator=(const SimpleFS &) = delete;
        SimpleFS(SimpleFS &&) noexcept;
        SimpleFS &operator=(SimpleFS &&) noexcept;

        /// @brief Opens file path with requested mode.
        /// @param path Absolute file path.
        /// @param mode Open mode flags.
        /// @return File descriptor on success or error code.
        [[nodiscard]] Result<FileDescriptor> open(const std::string &path, OpenMode mode) override;

        /// @brief Reads bytes from descriptor into buffer.
        /// @param fd Open descriptor.
        /// @param buffer Destination buffer.
        /// @return Number of bytes read or error code.
        [[nodiscard]] Result<std::size_t> read(FileDescriptor fd, std::span<std::byte> buffer) override;

        /// @brief Writes bytes from data into descriptor stream.
        /// @param fd Open descriptor.
        /// @param data Source bytes.
        /// @return Number of bytes written or error code.
        [[nodiscard]] Result<std::size_t> write(FileDescriptor fd, std::span<const std::byte> data) override;

        /// @brief Closes open descriptor.
        /// @param fd Descriptor to close.
        /// @return Ok on success; NotFound if descriptor is unknown.
        [[nodiscard]] Result<void> close(FileDescriptor fd) override;

        /// @brief Creates a directory at path.
        /// @param path Absolute directory path.
        /// @return Ok on success; AlreadyExists/NotFound/InvalidArgument otherwise.
        [[nodiscard]] Result<void> mkdir(const std::string &path) override;

        /// @brief Removes file or empty directory at path.
        /// @param path Absolute path.
        /// @return Ok on success; NotFound/InvalidState/PermissionDenied otherwise.
        [[nodiscard]] Result<void> remove(const std::string &path) override;

        /// @brief Lists child entries in directory.
        /// @param path Absolute directory path.
        /// @return Entries on success; error code otherwise.
        [[nodiscard]] Result<std::vector<DirectoryEntry>> listDir(const std::string &path) const override;

        /// @brief Returns metadata for path.
        /// @param path Absolute file or directory path.
        /// @return Metadata on success; NotFound otherwise.
        [[nodiscard]] Result<InodeInfo> stat(const std::string &path) const override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
