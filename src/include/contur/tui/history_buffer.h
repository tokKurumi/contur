/// @file history_buffer.h
/// @brief Bounded TUI history ring-buffer with cursor navigation semantics.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>

#include "contur/core/error.h"

#include "contur/tui/tui_models.h"

namespace contur {

    /// @brief Bounded history buffer used for UI-only playback navigation.
    class HistoryBuffer final
    {
        public:
        /// @brief Constructs history buffer with bounded capacity.
        /// @param capacity Maximum number of entries retained (0 normalizes to 1).
        explicit HistoryBuffer(std::size_t capacity);

        /// @brief Destroys history buffer.
        ~HistoryBuffer();

        /// @brief Copy construction is disabled.
        HistoryBuffer(const HistoryBuffer &) = delete;

        /// @brief Copy assignment is disabled.
        HistoryBuffer &operator=(const HistoryBuffer &) = delete;

        /// @brief Move-constructs buffer state.
        HistoryBuffer(HistoryBuffer &&) noexcept;

        /// @brief Move-assigns buffer state.
        HistoryBuffer &operator=(HistoryBuffer &&) noexcept;

        /// @brief Appends a snapshot entry and moves cursor to newest entry.
        /// @param entry New history entry.
        /// @return Ok on success.
        [[nodiscard]] Result<void> append(TuiHistoryEntry entry);

        /// @brief Moves cursor backward by N entries.
        /// @param step Number of entries to move backward.
        /// @return Ok on success, NotFound if movement exceeds available history.
        [[nodiscard]] Result<void> seekBackward(std::size_t step);

        /// @brief Moves cursor forward by N entries.
        /// @param step Number of entries to move forward.
        /// @return Ok on success, NotFound if movement exceeds available history.
        [[nodiscard]] Result<void> seekForward(std::size_t step);

        /// @brief Moves cursor to latest entry when history is non-empty.
        void moveToLatest() noexcept;

        /// @brief Returns current cursor entry.
        /// @return Entry reference when available, std::nullopt when buffer is empty.
        [[nodiscard]] std::optional<std::reference_wrapper<const TuiHistoryEntry>> current() const noexcept;

        /// @brief Returns latest entry.
        /// @return Entry reference when available, std::nullopt when buffer is empty.
        [[nodiscard]] std::optional<std::reference_wrapper<const TuiHistoryEntry>> latest() const noexcept;

        /// @brief Returns true when buffer has no entries.
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Returns number of retained entries.
        [[nodiscard]] std::size_t size() const noexcept;

        /// @brief Returns configured capacity.
        [[nodiscard]] std::size_t capacity() const noexcept;

        /// @brief Returns current cursor index in [0, size-1], or 0 when empty.
        [[nodiscard]] std::size_t cursor() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
