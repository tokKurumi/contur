/// @file console_sink.h
/// @brief Console trace sink implementation.

#pragma once

#include <memory>

#include "contur/tracing/trace_sink.h"

namespace contur {

    /// @brief Trace sink that writes formatted events to stdout.
    class ConsoleSink final : public ITraceSink
    {
        public:
        /// @brief Constructs a console sink.
        ConsoleSink();

        /// @brief Destroys console sink.
        ~ConsoleSink() override;

        /// @brief Copy construction is disabled.
        ConsoleSink(const ConsoleSink &) = delete;

        /// @brief Copy assignment is disabled.
        ConsoleSink &operator=(const ConsoleSink &) = delete;
        /// @brief Move-constructs sink state.
        ConsoleSink(ConsoleSink &&) noexcept;

        /// @brief Move-assigns sink state.
        ConsoleSink &operator=(ConsoleSink &&) noexcept;

        /// @copydoc ITraceSink::write
        void write(const TraceEvent &event) override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
