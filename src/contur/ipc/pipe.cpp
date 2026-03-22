/// @file pipe.cpp
/// @brief Pipe IPC channel implementation.

#include "contur/ipc/pipe.h"

#include <algorithm>
#include <deque>
#include <utility>

namespace contur {

    struct Pipe::Impl
    {
        std::string name;
        std::size_t capacity = 0;
        bool open = true;
        std::deque<std::byte> buffer;
    };

    Pipe::Pipe(std::string name, std::size_t capacity)
        : impl_(std::make_unique<Impl>())
    {
        impl_->name = std::move(name);
        impl_->capacity = std::max<std::size_t>(1, capacity);
    }

    Pipe::~Pipe() = default;
    Pipe::Pipe(Pipe &&) noexcept = default;
    Pipe &Pipe::operator=(Pipe &&) noexcept = default;

    Result<std::size_t> Pipe::write(std::span<const std::byte> data)
    {
        if (!impl_->open)
        {
            return Result<std::size_t>::error(ErrorCode::InvalidState);
        }
        if (data.empty())
        {
            return Result<std::size_t>::ok(0);
        }

        std::size_t available = impl_->capacity - impl_->buffer.size();
        if (available == 0)
        {
            return Result<std::size_t>::error(ErrorCode::BufferFull);
        }

        const std::size_t toWrite = std::min(available, data.size());
        for (std::size_t i = 0; i < toWrite; ++i)
        {
            impl_->buffer.push_back(data[i]);
        }

        return Result<std::size_t>::ok(toWrite);
    }

    Result<std::size_t> Pipe::read(std::span<std::byte> buffer)
    {
        if (!impl_->open)
        {
            return Result<std::size_t>::error(ErrorCode::InvalidState);
        }
        if (buffer.empty())
        {
            return Result<std::size_t>::ok(0);
        }
        if (impl_->buffer.empty())
        {
            return Result<std::size_t>::error(ErrorCode::BufferEmpty);
        }

        const std::size_t toRead = std::min(buffer.size(), impl_->buffer.size());
        for (std::size_t i = 0; i < toRead; ++i)
        {
            buffer[i] = impl_->buffer.front();
            impl_->buffer.pop_front();
        }

        return Result<std::size_t>::ok(toRead);
    }

    void Pipe::close()
    {
        impl_->open = false;
        impl_->buffer.clear();
    }

    bool Pipe::isOpen() const noexcept
    {
        return impl_->open;
    }

    std::string_view Pipe::name() const noexcept
    {
        return impl_->name;
    }

    std::size_t Pipe::capacity() const noexcept
    {
        return impl_->capacity;
    }

    std::size_t Pipe::size() const noexcept
    {
        return impl_->buffer.size();
    }

} // namespace contur
