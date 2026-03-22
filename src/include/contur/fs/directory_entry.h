/// @file directory_entry.h
/// @brief Directory entry representation for SimpleFS.

#pragma once

#include <string>

#include "contur/fs/inode.h"

namespace contur {

    /// @brief One child entry in a directory.
    struct DirectoryEntry
    {
        /// @brief Child name relative to parent directory.
        std::string name;

        /// @brief Child inode identifier.
        InodeId inodeId = INVALID_INODE_ID;

        /// @brief Child inode type.
        InodeType type = InodeType::File;
    };

} // namespace contur
