/// @file buffer_sink.cpp
/// @brief In-memory trace sink implementation.

#include "contur/tracing/buffer_sink.h"

#include <mutex>
#include <utility>

namespace contur {

    struct BufferSink::Impl
    {
        mutable std::mutex mutex;
        std::vector<TraceEvent> events;
    };

    BufferSink::BufferSink()
        : impl_(std::make_unique<Impl>())
    {}

    BufferSink::~BufferSink() = default;
    BufferSink::BufferSink(BufferSink &&) noexcept = default;
    BufferSink &BufferSink::operator=(BufferSink &&) noexcept = default;

    void BufferSink::write(const TraceEvent &event)
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        impl_->events.push_back(event);
    }

    std::vector<TraceEvent> BufferSink::snapshot() const
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->events;
    }

    void BufferSink::clear()
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        impl_->events.clear();
    }

    std::size_t BufferSink::size() const
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->events.size();
    }

} // namespace contur
