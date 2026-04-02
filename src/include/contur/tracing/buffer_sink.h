/// @file buffer_sink.h
/// @brief In-memory trace sink implementation for tests and diagnostics.

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "contur/tracing/trace_sink.h"

namespace contur {

    /// @brief Trace sink that stores events in memory.
    class BufferSink final : public ITraceSink
    {
        public:
        /// @brief Constructs an empty buffer sink.
        BufferSink();

        /// @brief Destroys buffer sink.
        ~BufferSink() override;

        /// @brief Copy construction is disabled.
        BufferSink(const BufferSink &) = delete;

        /// @brief Copy assignment is disabled.
        BufferSink &operator=(const BufferSink &) = delete;
        /// @brief Move-constructs sink state.
        BufferSink(BufferSink &&) noexcept;

        /// @brief Move-assigns sink state.
        BufferSink &operator=(BufferSink &&) noexcept;

        /// @copydoc ITraceSink::write
        void write(const TraceEvent &event) override;

        /// @brief Returns a copy of all captured trace events.
        [[nodiscard]] std::vector<TraceEvent> snapshot() const;

        /// @brief Removes all buffered trace events.
        void clear();

        /// @brief Returns number of currently buffered events.
        [[nodiscard]] std::size_t size() const;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
