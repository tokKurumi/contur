/// @file lru_replacement.h
/// @brief LRU page replacement policy — evicts the least recently used page.

#pragma once

#include <memory>

#include "contur/memory/i_page_replacement.h"

namespace contur {

    /// @brief Least Recently Used page replacement.
    ///
    /// Tracks access order via timestamps. On eviction, selects the frame
    /// that has not been accessed for the longest time.
    class LruReplacement final : public IPageReplacementPolicy
    {
        public:
        /// @brief Creates an LRU replacement policy with empty history state.
        LruReplacement();

        /// @brief Destroys LRU replacement policy.
        ~LruReplacement() override;

        /// @brief Copy construction is disabled.
        LruReplacement(const LruReplacement &) = delete;

        /// @brief Copy assignment is disabled.
        LruReplacement &operator=(const LruReplacement &) = delete;
        /// @brief Move-constructs policy state.
        LruReplacement(LruReplacement &&) noexcept;

        /// @brief Move-assigns policy state.
        LruReplacement &operator=(LruReplacement &&) noexcept;

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
