/// @file trace_sink.h
/// @brief Trace sink interface for trace event consumers.

#pragma once

#include "contur/tracing/trace_event.h"

namespace contur {

    /// @brief Sink interface that receives structured trace events.
    class ITraceSink
    {
        public:
        /// @brief Virtual destructor for polymorphic cleanup.
        virtual ~ITraceSink() = default;

        /// @brief Writes one trace event to the sink.
        /// @param event Trace event record.
        virtual void write(const TraceEvent &event) = 0;
    };

} // namespace contur
