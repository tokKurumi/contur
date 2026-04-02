/// @file file_sink.h
/// @brief File trace sink implementation.

#pragma once

#include <memory>
#include <string>

#include "contur/tracing/trace_sink.h"

namespace contur {

    /// @brief Trace sink that appends events to a text file.
    class FileSink final : public ITraceSink
    {
        public:
        /// @brief Constructs a file sink for the given path.
        /// @param filePath Output log file path.
        explicit FileSink(std::string filePath);

        /// @brief Destroys file sink.
        ~FileSink() override;

        /// @brief Copy construction is disabled.
        FileSink(const FileSink &) = delete;

        /// @brief Copy assignment is disabled.
        FileSink &operator=(const FileSink &) = delete;
        /// @brief Move-constructs sink state.
        FileSink(FileSink &&) noexcept;

        /// @brief Move-assigns sink state.
        FileSink &operator=(FileSink &&) noexcept;

        /// @copydoc ITraceSink::write
        void write(const TraceEvent &event) override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
