/// @file syscall_table.cpp
/// @brief SyscallTable implementation.

#include "contur/syscall/syscall_table.h"

#include <unordered_map>
#include <utility>

#include "contur/process/process_image.h"

namespace contur {

    namespace {

        struct SyscallIdHash
        {
            std::size_t operator()(SyscallId id) const noexcept
            {
                return static_cast<std::size_t>(id);
            }
        };

    } // namespace

    struct SyscallTable::Impl
    {
        std::unordered_map<SyscallId, HandlerFn, SyscallIdHash> handlers;
    };

    SyscallTable::SyscallTable()
        : impl_(std::make_unique<Impl>())
    {}

    SyscallTable::~SyscallTable() = default;
    SyscallTable::SyscallTable(SyscallTable &&) noexcept = default;
    SyscallTable &SyscallTable::operator=(SyscallTable &&) noexcept = default;

    Result<void> SyscallTable::registerHandler(SyscallId id, HandlerFn handler)
    {
        if (!handler)
        {
            return Result<void>::error(ErrorCode::InvalidArgument);
        }

        impl_->handlers[id] = std::move(handler);
        return Result<void>::ok();
    }

    Result<void> SyscallTable::registerHandler(SyscallId id, ISyscallHandler &handler)
    {
        return registerHandler(id, [&handler, id](std::span<const RegisterValue> args, ProcessImage &caller) {
            return handler.handle(id, args, caller);
        });
    }

    Result<void> SyscallTable::unregisterHandler(SyscallId id)
    {
        auto erased = impl_->handlers.erase(id);
        if (erased == 0)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        return Result<void>::ok();
    }

    Result<RegisterValue>
    SyscallTable::dispatch(SyscallId id, std::span<const RegisterValue> args, ProcessImage &caller) const
    {
        auto it = impl_->handlers.find(id);
        if (it == impl_->handlers.end())
        {
            return Result<RegisterValue>::error(ErrorCode::NotFound);
        }

        return it->second(args, caller);
    }

    bool SyscallTable::hasHandler(SyscallId id) const noexcept
    {
        return impl_->handlers.find(id) != impl_->handlers.end();
    }

    std::size_t SyscallTable::handlerCount() const noexcept
    {
        return impl_->handlers.size();
    }

} // namespace contur
