/// @file history_buffer.cpp
/// @brief HistoryBuffer implementation.

#include "contur/tui/history_buffer.h"

#include <deque>
#include <utility>

namespace contur {

    struct HistoryBuffer::Impl
    {
        std::size_t capacity = 1;
        std::deque<TuiHistoryEntry> entries;
        std::size_t cursor = 0;

        explicit Impl(std::size_t requestedCapacity)
            : capacity(requestedCapacity == 0 ? 1 : requestedCapacity)
        {}
    };

    HistoryBuffer::HistoryBuffer(std::size_t capacity)
        : impl_(std::make_unique<Impl>(capacity))
    {}

    HistoryBuffer::~HistoryBuffer() = default;
    HistoryBuffer::HistoryBuffer(HistoryBuffer &&) noexcept = default;
    HistoryBuffer &HistoryBuffer::operator=(HistoryBuffer &&) noexcept = default;

    Result<void> HistoryBuffer::append(TuiHistoryEntry entry)
    {
        if (!impl_)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        if (impl_->entries.size() == impl_->capacity)
        {
            impl_->entries.pop_front();
            if (impl_->cursor > 0)
            {
                --impl_->cursor;
            }
        }

        impl_->entries.push_back(std::move(entry));
        impl_->cursor = impl_->entries.empty() ? 0 : (impl_->entries.size() - 1);
        return Result<void>::ok();
    }

    Result<void> HistoryBuffer::seekBackward(std::size_t step)
    {
        if (!impl_)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        if (impl_->entries.empty())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        if (step > impl_->cursor)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        impl_->cursor -= step;
        return Result<void>::ok();
    }

    Result<void> HistoryBuffer::seekForward(std::size_t step)
    {
        if (!impl_)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }

        if (impl_->entries.empty())
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        std::size_t maxForward = (impl_->entries.size() - 1) - impl_->cursor;
        if (step > maxForward)
        {
            return Result<void>::error(ErrorCode::NotFound);
        }

        impl_->cursor += step;
        return Result<void>::ok();
    }

    void HistoryBuffer::moveToLatest() noexcept
    {
        if (!impl_ || impl_->entries.empty())
        {
            return;
        }
        impl_->cursor = impl_->entries.size() - 1;
    }

    std::optional<std::reference_wrapper<const TuiHistoryEntry>> HistoryBuffer::current() const noexcept
    {
        if (!impl_ || impl_->entries.empty())
        {
            return std::nullopt;
        }
        return std::cref(impl_->entries[impl_->cursor]);
    }

    std::optional<std::reference_wrapper<const TuiHistoryEntry>> HistoryBuffer::latest() const noexcept
    {
        if (!impl_ || impl_->entries.empty())
        {
            return std::nullopt;
        }
        return std::cref(impl_->entries.back());
    }

    bool HistoryBuffer::empty() const noexcept
    {
        if (!impl_)
        {
            return true;
        }
        return impl_->entries.empty();
    }

    std::size_t HistoryBuffer::size() const noexcept
    {
        if (!impl_)
        {
            return 0;
        }
        return impl_->entries.size();
    }

    std::size_t HistoryBuffer::capacity() const noexcept
    {
        if (!impl_)
        {
            return 0;
        }
        return impl_->capacity;
    }

    std::size_t HistoryBuffer::cursor() const noexcept
    {
        if (!impl_ || impl_->entries.empty())
        {
            return 0;
        }
        return impl_->cursor;
    }

} // namespace contur
