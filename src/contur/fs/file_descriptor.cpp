/// @file file_descriptor.cpp
/// @brief FileDescriptorTable implementation.

#include "contur/fs/file_descriptor.h"

#include <unordered_map>

namespace contur {

    struct FileDescriptorTable::Impl
    {
        std::unordered_map<std::int32_t, OpenFileState> entries;
        std::int32_t nextFd = 0;
    };

    FileDescriptorTable::FileDescriptorTable()
        : impl_(std::make_unique<Impl>())
    {}

    FileDescriptorTable::~FileDescriptorTable() = default;
    FileDescriptorTable::FileDescriptorTable(FileDescriptorTable &&) noexcept = default;
    FileDescriptorTable &FileDescriptorTable::operator=(FileDescriptorTable &&) noexcept = default;

    Result<FileDescriptor> FileDescriptorTable::open(OpenFileState state)
    {
        const FileDescriptor fd{impl_->nextFd++};
        impl_->entries.emplace(fd.value, state);
        return Result<FileDescriptor>::ok(fd);
    }

    Result<void> FileDescriptorTable::close(FileDescriptor fd)
    {
        const auto erased = impl_->entries.erase(fd.value);
        if (erased == 0)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        return Result<void>::ok();
    }

    Result<OpenFileState> FileDescriptorTable::get(FileDescriptor fd) const
    {
        auto it = impl_->entries.find(fd.value);
        if (it == impl_->entries.end())
        {
            return Result<OpenFileState>::error(ErrorCode::NotFound);
        }

        return Result<OpenFileState>::ok(it->second);
    }

    Result<void> FileDescriptorTable::set(FileDescriptor fd, OpenFileState state)
    {
        auto it = impl_->entries.find(fd.value);
        if (it == impl_->entries.end())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        it->second = state;
        return Result<void>::ok();
    }

    bool FileDescriptorTable::contains(FileDescriptor fd) const noexcept
    {
        return impl_->entries.find(fd.value) != impl_->entries.end();
    }

    std::size_t FileDescriptorTable::openCount() const noexcept
    {
        return impl_->entries.size();
    }

} // namespace contur
