/// @file clock_replacement.h
/// @brief Clock (Second Chance) page replacement policy.

#pragma once

#include <memory>

#include "contur/memory/i_page_replacement.h"

namespace contur {

    /// @brief Clock (Second Chance) page replacement.
    ///
    /// Uses a circular buffer and reference bits. On eviction, scans frames:
    /// - If reference bit is set, clear it and advance (give a "second chance")
    /// - If reference bit is clear, select this frame as victim
    class ClockReplacement final : public IPageReplacementPolicy
    {
        public:
        /// @brief Creates a clock replacement policy with empty tracking state.
        ClockReplacement();

        /// @brief Destroys clock replacement policy.
        ~ClockReplacement() override;

        /// @brief Copy construction is disabled.
        ClockReplacement(const ClockReplacement &) = delete;

        /// @brief Copy assignment is disabled.
        ClockReplacement &operator=(const ClockReplacement &) = delete;
        /// @brief Move-constructs policy state.
        ClockReplacement(ClockReplacement &&) noexcept;

        /// @brief Move-assigns policy state.
        ClockReplacement &operator=(ClockReplacement &&) noexcept;

        /// @copydoc IPageReplacementPolicy::name
        [[nodiscard]] std::string_view name() const noexcept override;

        /// @copydoc IPageReplacementPolicy::selectVictim
        [[nodiscard]] FrameId selectVictim(const PageTable &pageTable) override;

        /// @copydoc IPageReplacementPolicy::onAccess
        void onAccess(FrameId frame) override;

        /// @copydoc IPageReplacementPolicy::onLoad
        void onLoad(FrameId frame) override;

        /// @copydoc IPageReplacementPolicy::reset
        void reset() override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
