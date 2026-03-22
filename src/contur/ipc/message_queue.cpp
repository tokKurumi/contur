/// @file message_queue.cpp
/// @brief MessageQueue IPC channel implementation.

#include "contur/ipc/message_queue.h"

#include <algorithm>
#include <deque>
#include <utility>

namespace contur {

    struct MessageQueue::Impl
    {
        std::string name;
        std::size_t maxMessages = 64;
        bool priorityMode = false;
        bool open = true;
        std::deque<Message> queue;
    };

    MessageQueue::MessageQueue(std::string name, std::size_t maxMessages, bool priorityMode)
        : impl_(std::make_unique<Impl>())
    {
        impl_->name = std::move(name);
        impl_->maxMessages = std::max<std::size_t>(1, maxMessages);
        impl_->priorityMode = priorityMode;
    }

    MessageQueue::~MessageQueue() = default;
    MessageQueue::MessageQueue(MessageQueue &&) noexcept = default;
    MessageQueue &MessageQueue::operator=(MessageQueue &&) noexcept = default;

    Result<std::size_t> MessageQueue::write(std::span<const std::byte> data)
    {
        Message message;
        message.payload.assign(data.begin(), data.end());

        auto sendResult = send(message);
        if (sendResult.isError())
        {
            return Result<std::size_t>::error(sendResult.errorCode());
        }

        return Result<std::size_t>::ok(data.size());
    }

    Result<std::size_t> MessageQueue::read(std::span<std::byte> buffer)
    {
        auto receiveResult = receive();
        if (receiveResult.isError())
        {
            return Result<std::size_t>::error(receiveResult.errorCode());
        }

        const Message &message = receiveResult.value();
        const std::size_t toCopy = std::min(buffer.size(), message.payload.size());
        std::copy_n(message.payload.begin(), toCopy, buffer.begin());
        return Result<std::size_t>::ok(toCopy);
    }

    void MessageQueue::close()
    {
        impl_->open = false;
        impl_->queue.clear();
    }

    bool MessageQueue::isOpen() const noexcept
    {
        return impl_->open;
    }

    std::string_view MessageQueue::name() const noexcept
    {
        return impl_->name;
    }

    Result<void> MessageQueue::send(const Message &message)
    {
        if (!impl_->open)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }
        if (impl_->queue.size() >= impl_->maxMessages)
        {
            return Result<void>::error(ErrorCode::BufferFull);
        }

        if (impl_->priorityMode)
        {
            auto it = std::find_if(impl_->queue.begin(), impl_->queue.end(), [&](const Message &existing) {
                return message.priority > existing.priority;
            });
            impl_->queue.insert(it, message);
        }
        else
        {
            impl_->queue.push_back(message);
        }

        return Result<void>::ok();
    }

    Result<Message> MessageQueue::receive()
    {
        if (!impl_->open)
        {
            return Result<Message>::error(ErrorCode::InvalidState);
        }
        if (impl_->queue.empty())
        {
            return Result<Message>::error(ErrorCode::BufferEmpty);
        }

        Message message = std::move(impl_->queue.front());
        impl_->queue.pop_front();
        return Result<Message>::ok(std::move(message));
    }

    std::size_t MessageQueue::size() const noexcept
    {
        return impl_->queue.size();
    }

    std::size_t MessageQueue::maxMessages() const noexcept
    {
        return impl_->maxMessages;
    }

    bool MessageQueue::isPriorityMode() const noexcept
    {
        return impl_->priorityMode;
    }

} // namespace contur
