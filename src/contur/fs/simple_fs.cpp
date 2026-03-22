/// @file simple_fs.cpp
/// @brief SimpleFS implementation.

#include "contur/fs/simple_fs.h"

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#include "contur/fs/block_allocator.h"

namespace contur {

    namespace {

        constexpr InodeId ROOT_INODE_ID = 1;

        struct PathParts
        {
            std::vector<std::string> tokens;
        };

        [[nodiscard]] auto splitPath(const std::string &path) -> Result<PathParts>
        {
            if (path.empty() || path[0] != '/')
            {
                return Result<PathParts>::error(ErrorCode::InvalidArgument);
            }

            PathParts parts;
            std::size_t start = 1;

            while (start < path.size())
            {
                while (start < path.size() && path[start] == '/')
                {
                    ++start;
                }
                if (start >= path.size())
                {
                    break;
                }

                const std::size_t slashPos = path.find('/', start);
                if (slashPos == std::string::npos)
                {
                    parts.tokens.push_back(path.substr(start));
                    break;
                }

                parts.tokens.push_back(path.substr(start, slashPos - start));
                start = slashPos + 1;
            }

            for (const auto &token : parts.tokens)
            {
                if (token.empty() || token == "." || token == "..")
                {
                    return Result<PathParts>::error(ErrorCode::InvalidArgument);
                }
            }

            return Result<PathParts>::ok(std::move(parts));
        }

    } // namespace

    struct SimpleFS::Impl
    {
        struct Node
        {
            Inode inode;
            std::unordered_map<std::string, InodeId> children;
        };

        std::size_t blockSize;
        std::vector<std::vector<std::byte>> disk;
        BlockAllocator allocator;
        FileDescriptorTable fdTable;
        std::unordered_map<InodeId, Node> nodes;
        InodeId nextInodeId = ROOT_INODE_ID + 1;
        Tick tick = 0;

        Impl(std::size_t totalBlocks, std::size_t blockSizeArg)
            : blockSize(std::max<std::size_t>(1, blockSizeArg))
            , disk(totalBlocks, std::vector<std::byte>(std::max<std::size_t>(1, blockSizeArg), std::byte{0}))
            , allocator(totalBlocks)
        {
            Node root;
            root.inode.id = ROOT_INODE_ID;
            root.inode.type = InodeType::Directory;
            root.inode.size = 0;
            root.inode.createdAt = tick;
            root.inode.modifiedAt = tick;
            nodes.emplace(ROOT_INODE_ID, std::move(root));
        }

        [[nodiscard]] Result<InodeId> resolvePath(const std::string &path) const
        {
            auto split = splitPath(path);
            if (split.isError())
            {
                return Result<InodeId>::error(split.errorCode());
            }

            InodeId current = ROOT_INODE_ID;
            for (const auto &token : split.value().tokens)
            {
                auto nodeIt = nodes.find(current);
                if (nodeIt == nodes.end() || nodeIt->second.inode.type != InodeType::Directory)
                {
                    return Result<InodeId>::error(ErrorCode::NotFound);
                }

                const auto childIt = nodeIt->second.children.find(token);
                if (childIt == nodeIt->second.children.end())
                {
                    return Result<InodeId>::error(ErrorCode::NotFound);
                }

                current = childIt->second;
            }

            return Result<InodeId>::ok(current);
        }

        [[nodiscard]] Result<std::pair<InodeId, std::string>> resolveParent(const std::string &path) const
        {
            auto split = splitPath(path);
            if (split.isError())
            {
                return Result<std::pair<InodeId, std::string>>::error(split.errorCode());
            }

            const auto &tokens = split.value().tokens;
            if (tokens.empty())
            {
                return Result<std::pair<InodeId, std::string>>::error(ErrorCode::InvalidArgument);
            }

            InodeId current = ROOT_INODE_ID;
            for (std::size_t i = 0; i + 1 < tokens.size(); ++i)
            {
                auto nodeIt = nodes.find(current);
                if (nodeIt == nodes.end() || nodeIt->second.inode.type != InodeType::Directory)
                {
                    return Result<std::pair<InodeId, std::string>>::error(ErrorCode::NotFound);
                }

                const auto childIt = nodeIt->second.children.find(tokens[i]);
                if (childIt == nodeIt->second.children.end())
                {
                    return Result<std::pair<InodeId, std::string>>::error(ErrorCode::NotFound);
                }

                current = childIt->second;
            }

            return Result<std::pair<InodeId, std::string>>::ok({current, tokens.back()});
        }

        [[nodiscard]] Result<void> ensureFileBlocks(Node &node, std::size_t requiredBytes)
        {
            const std::size_t requiredBlocks = requiredBytes == 0 ? 0 : (requiredBytes + blockSize - 1) / blockSize;

            while (node.inode.blocks.size() < requiredBlocks)
            {
                auto alloc = allocator.allocate();
                if (alloc.isError())
                {
                    return Result<void>::error(alloc.errorCode());
                }
                node.inode.blocks.push_back(alloc.value());
            }

            while (node.inode.blocks.size() > requiredBlocks)
            {
                const std::size_t blockIndex = node.inode.blocks.back();
                auto freed = allocator.free(blockIndex);
                if (freed.isError())
                {
                    return Result<void>::error(freed.errorCode());
                }
                std::fill(disk[blockIndex].begin(), disk[blockIndex].end(), std::byte{0});
                node.inode.blocks.pop_back();
            }

            return Result<void>::ok();
        }

        [[nodiscard]] Result<std::size_t> writeToNode(Node &node, std::size_t offset, std::span<const std::byte> data)
        {
            if (data.empty())
            {
                return Result<std::size_t>::ok(0);
            }

            const std::size_t endOffset = offset + data.size();
            auto ensure = ensureFileBlocks(node, std::max(node.inode.size, endOffset));
            if (ensure.isError())
            {
                return Result<std::size_t>::error(ensure.errorCode());
            }

            std::size_t written = 0;
            while (written < data.size())
            {
                const std::size_t absolute = offset + written;
                const std::size_t blockSlot = absolute / blockSize;
                const std::size_t blockOffset = absolute % blockSize;
                const std::size_t chunk = std::min(blockSize - blockOffset, data.size() - written);

                const std::size_t diskBlock = node.inode.blocks[blockSlot];
                std::copy_n(
                    data.begin() + static_cast<std::ptrdiff_t>(written),
                    chunk,
                    disk[diskBlock].begin() + static_cast<std::ptrdiff_t>(blockOffset)
                );
                written += chunk;
            }

            node.inode.size = std::max(node.inode.size, endOffset);
            node.inode.modifiedAt = ++tick;
            return Result<std::size_t>::ok(written);
        }

        [[nodiscard]] Result<std::size_t>
        readFromNode(const Node &node, std::size_t offset, std::span<std::byte> buffer) const
        {
            if (buffer.empty())
            {
                return Result<std::size_t>::ok(0);
            }

            if (offset >= node.inode.size)
            {
                return Result<std::size_t>::ok(0);
            }

            const std::size_t remaining = node.inode.size - offset;
            const std::size_t totalRead = std::min(buffer.size(), remaining);

            std::size_t read = 0;
            while (read < totalRead)
            {
                const std::size_t absolute = offset + read;
                const std::size_t blockSlot = absolute / blockSize;
                const std::size_t blockOffset = absolute % blockSize;
                const std::size_t chunk = std::min(blockSize - blockOffset, totalRead - read);

                const std::size_t diskBlock = node.inode.blocks[blockSlot];
                std::copy_n(
                    disk[diskBlock].begin() + static_cast<std::ptrdiff_t>(blockOffset),
                    chunk,
                    buffer.begin() + static_cast<std::ptrdiff_t>(read)
                );
                read += chunk;
            }

            return Result<std::size_t>::ok(totalRead);
        }

        [[nodiscard]] InodeInfo toInodeInfo(const Inode &inode) const
        {
            return InodeInfo{inode.id, inode.type, inode.size, inode.blocks.size(), inode.createdAt, inode.modifiedAt};
        }
    };

    SimpleFS::SimpleFS(std::size_t totalBlocks, std::size_t blockSize)
        : impl_(std::make_unique<Impl>(std::max<std::size_t>(1, totalBlocks), blockSize))
    {}

    SimpleFS::~SimpleFS() = default;
    SimpleFS::SimpleFS(SimpleFS &&) noexcept = default;
    SimpleFS &SimpleFS::operator=(SimpleFS &&) noexcept = default;

    Result<FileDescriptor> SimpleFS::open(const std::string &path, OpenMode mode)
    {
        if (!hasOpenMode(mode, OpenMode::Read) && !hasOpenMode(mode, OpenMode::Write))
        {
            return Result<FileDescriptor>::error(ErrorCode::InvalidArgument);
        }

        auto inodeResult = impl_->resolvePath(path);
        if (inodeResult.isError())
        {
            if (!hasOpenMode(mode, OpenMode::Create))
            {
                return Result<FileDescriptor>::error(inodeResult.errorCode());
            }

            auto parentResult = impl_->resolveParent(path);
            if (parentResult.isError())
            {
                return Result<FileDescriptor>::error(parentResult.errorCode());
            }

            const InodeId parentId = parentResult.value().first;
            const std::string &name = parentResult.value().second;

            auto parentIt = impl_->nodes.find(parentId);
            if (parentIt == impl_->nodes.end() || parentIt->second.inode.type != InodeType::Directory)
            {
                return Result<FileDescriptor>::error(ErrorCode::NotFound);
            }

            if (parentIt->second.children.find(name) != parentIt->second.children.end())
            {
                return Result<FileDescriptor>::error(ErrorCode::AlreadyExists);
            }

            Impl::Node fileNode;
            fileNode.inode.id = impl_->nextInodeId++;
            fileNode.inode.type = InodeType::File;
            fileNode.inode.parent = parentId;
            fileNode.inode.createdAt = ++impl_->tick;
            fileNode.inode.modifiedAt = impl_->tick;

            const InodeId newId = fileNode.inode.id;
            impl_->nodes.emplace(newId, std::move(fileNode));
            parentIt->second.children.emplace(name, newId);
            parentIt->second.inode.size = parentIt->second.children.size();
            parentIt->second.inode.modifiedAt = ++impl_->tick;

            inodeResult = Result<InodeId>::ok(newId);
        }

        auto nodeIt = impl_->nodes.find(inodeResult.value());
        if (nodeIt == impl_->nodes.end())
        {
            return Result<FileDescriptor>::error(ErrorCode::NotFound);
        }

        if (nodeIt->second.inode.type != InodeType::File)
        {
            return Result<FileDescriptor>::error(ErrorCode::InvalidState);
        }

        if (hasOpenMode(mode, OpenMode::Truncate))
        {
            auto resize = impl_->ensureFileBlocks(nodeIt->second, 0);
            if (resize.isError())
            {
                return Result<FileDescriptor>::error(resize.errorCode());
            }
            nodeIt->second.inode.size = 0;
            nodeIt->second.inode.modifiedAt = ++impl_->tick;
        }

        OpenFileState state;
        state.inodeId = nodeIt->second.inode.id;
        state.mode = mode;
        state.offset = hasOpenMode(mode, OpenMode::Append) ? nodeIt->second.inode.size : 0;

        return impl_->fdTable.open(state);
    }

    Result<std::size_t> SimpleFS::read(FileDescriptor fd, std::span<std::byte> buffer)
    {
        auto stateResult = impl_->fdTable.get(fd);
        if (stateResult.isError())
        {
            return Result<std::size_t>::error(stateResult.errorCode());
        }

        OpenFileState state = stateResult.value();
        if (!hasOpenMode(state.mode, OpenMode::Read))
        {
            return Result<std::size_t>::error(ErrorCode::PermissionDenied);
        }

        auto nodeIt = impl_->nodes.find(state.inodeId);
        if (nodeIt == impl_->nodes.end())
        {
            return Result<std::size_t>::error(ErrorCode::NotFound);
        }

        auto readResult = impl_->readFromNode(nodeIt->second, state.offset, buffer);
        if (readResult.isError())
        {
            return Result<std::size_t>::error(readResult.errorCode());
        }

        state.offset += readResult.value();
        auto setResult = impl_->fdTable.set(fd, state);
        if (setResult.isError())
        {
            return Result<std::size_t>::error(setResult.errorCode());
        }

        return readResult;
    }

    Result<std::size_t> SimpleFS::write(FileDescriptor fd, std::span<const std::byte> data)
    {
        auto stateResult = impl_->fdTable.get(fd);
        if (stateResult.isError())
        {
            return Result<std::size_t>::error(stateResult.errorCode());
        }

        OpenFileState state = stateResult.value();
        if (!hasOpenMode(state.mode, OpenMode::Write))
        {
            return Result<std::size_t>::error(ErrorCode::PermissionDenied);
        }

        auto nodeIt = impl_->nodes.find(state.inodeId);
        if (nodeIt == impl_->nodes.end())
        {
            return Result<std::size_t>::error(ErrorCode::NotFound);
        }

        if (hasOpenMode(state.mode, OpenMode::Append))
        {
            state.offset = nodeIt->second.inode.size;
        }

        auto writeResult = impl_->writeToNode(nodeIt->second, state.offset, data);
        if (writeResult.isError())
        {
            return Result<std::size_t>::error(writeResult.errorCode());
        }

        state.offset += writeResult.value();
        auto setResult = impl_->fdTable.set(fd, state);
        if (setResult.isError())
        {
            return Result<std::size_t>::error(setResult.errorCode());
        }

        return writeResult;
    }

    Result<void> SimpleFS::close(FileDescriptor fd)
    {
        return impl_->fdTable.close(fd);
    }

    Result<void> SimpleFS::mkdir(const std::string &path)
    {
        auto parentResult = impl_->resolveParent(path);
        if (parentResult.isError())
        {
            return Result<void>::error(parentResult.errorCode());
        }

        const InodeId parentId = parentResult.value().first;
        const std::string &name = parentResult.value().second;

        auto parentIt = impl_->nodes.find(parentId);
        if (parentIt == impl_->nodes.end() || parentIt->second.inode.type != InodeType::Directory)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        if (parentIt->second.children.find(name) != parentIt->second.children.end())
        {
            return Result<void>::error(ErrorCode::AlreadyExists);
        }

        Impl::Node dirNode;
        dirNode.inode.id = impl_->nextInodeId++;
        dirNode.inode.type = InodeType::Directory;
        dirNode.inode.parent = parentId;
        dirNode.inode.createdAt = ++impl_->tick;
        dirNode.inode.modifiedAt = impl_->tick;

        const InodeId newId = dirNode.inode.id;
        impl_->nodes.emplace(newId, std::move(dirNode));
        parentIt->second.children.emplace(name, newId);
        parentIt->second.inode.size = parentIt->second.children.size();
        parentIt->second.inode.modifiedAt = ++impl_->tick;

        return Result<void>::ok();
    }

    Result<void> SimpleFS::remove(const std::string &path)
    {
        auto parentResult = impl_->resolveParent(path);
        if (parentResult.isError())
        {
            return Result<void>::error(parentResult.errorCode());
        }

        const InodeId parentId = parentResult.value().first;
        const std::string &name = parentResult.value().second;

        auto parentIt = impl_->nodes.find(parentId);
        if (parentIt == impl_->nodes.end() || parentIt->second.inode.type != InodeType::Directory)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        auto childIt = parentIt->second.children.find(name);
        if (childIt == parentIt->second.children.end())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        auto nodeIt = impl_->nodes.find(childIt->second);
        if (nodeIt == impl_->nodes.end())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        if (nodeIt->second.inode.id == ROOT_INODE_ID)
        {
            return Result<void>::error(ErrorCode::PermissionDenied);
        }

        if (nodeIt->second.inode.type == InodeType::Directory && !nodeIt->second.children.empty())
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        if (nodeIt->second.inode.type == InodeType::File)
        {
            for (const std::size_t blockIndex : nodeIt->second.inode.blocks)
            {
                auto freeResult = impl_->allocator.free(blockIndex);
                if (freeResult.isError())
                {
                    return Result<void>::error(freeResult.errorCode());
                }
                std::fill(impl_->disk[blockIndex].begin(), impl_->disk[blockIndex].end(), std::byte{0});
            }
        }

        impl_->nodes.erase(nodeIt);
        parentIt->second.children.erase(childIt);
        parentIt->second.inode.size = parentIt->second.children.size();
        parentIt->second.inode.modifiedAt = ++impl_->tick;

        return Result<void>::ok();
    }

    Result<std::vector<DirectoryEntry>> SimpleFS::listDir(const std::string &path) const
    {
        auto inodeResult = impl_->resolvePath(path);
        if (inodeResult.isError())
        {
            return Result<std::vector<DirectoryEntry>>::error(inodeResult.errorCode());
        }

        auto nodeIt = impl_->nodes.find(inodeResult.value());
        if (nodeIt == impl_->nodes.end())
        {
            return Result<std::vector<DirectoryEntry>>::error(ErrorCode::NotFound);
        }

        if (nodeIt->second.inode.type != InodeType::Directory)
        {
            return Result<std::vector<DirectoryEntry>>::error(ErrorCode::InvalidState);
        }

        std::vector<DirectoryEntry> entries;
        entries.reserve(nodeIt->second.children.size());

        for (const auto &[name, inodeId] : nodeIt->second.children)
        {
            auto childIt = impl_->nodes.find(inodeId);
            if (childIt == impl_->nodes.end())
            {
                continue;
            }
            entries.push_back(DirectoryEntry{name, inodeId, childIt->second.inode.type});
        }

        std::sort(entries.begin(), entries.end(), [](const DirectoryEntry &lhs, const DirectoryEntry &rhs) {
            return lhs.name < rhs.name;
        });

        return Result<std::vector<DirectoryEntry>>::ok(std::move(entries));
    }

    Result<InodeInfo> SimpleFS::stat(const std::string &path) const
    {
        auto inodeResult = impl_->resolvePath(path);
        if (inodeResult.isError())
        {
            return Result<InodeInfo>::error(inodeResult.errorCode());
        }

        auto nodeIt = impl_->nodes.find(inodeResult.value());
        if (nodeIt == impl_->nodes.end())
        {
            return Result<InodeInfo>::error(ErrorCode::NotFound);
        }

        return Result<InodeInfo>::ok(impl_->toInodeInfo(nodeIt->second.inode));
    }

} // namespace contur
