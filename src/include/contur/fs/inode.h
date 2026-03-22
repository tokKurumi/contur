/// @file inode.h
/// @brief Inode types for the SimpleFS file-system simulation.

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "contur/core/types.h"

namespace contur {

    /// @brief Unique inode identifier type.
    using InodeId = std::uint32_t;

    /// @brief Sentinel value for an invalid inode identifier.
    constexpr InodeId INVALID_INODE_ID = 0;

    /// @brief Type of filesystem node.
    enum class InodeType : std::uint8_t
    {
        File,
        Directory,
    };

    /// @brief Persistent metadata describing a filesystem node.
    struct Inode
    {
        /// @brief Unique inode identifier.
        InodeId id = INVALID_INODE_ID;

        /// @brief Node type (file or directory).
        InodeType type = InodeType::File;

        /// @brief Logical file size in bytes. For directories, this is entry count.
        std::size_t size = 0;

        /// @brief Allocated data block indices owned by this inode.
        std::vector<std::size_t> blocks;

        /// @brief Parent inode, if any.
        std::optional<InodeId> parent;

        /// @brief Creation timestamp in simulation ticks.
        Tick createdAt = 0;

        /// @brief Last-modification timestamp in simulation ticks.
        Tick modifiedAt = 0;
    };

} // namespace contur
