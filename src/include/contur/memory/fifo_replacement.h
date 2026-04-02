/// @file fifo_replacement.h
/// @brief FIFO page replacement policy — evicts the oldest loaded page.

#pragma once

#include <memory>

#include "contur/memory/i_page_replacement.h"

namespace contur {

    /// @brief First-In First-Out page replacement.
    ///
    /// Maintains a queue of loaded frames in order of arrival.
    /// On eviction, selects the frame that was loaded earliest.
    class FifoReplacement final : public IPageReplacementPolicy
    {
        public:
        /// @brief Creates a FIFO replacement policy with empty queue state.
        FifoReplacement();

        /// @brief Destroys FIFO replacement policy.
        ~FifoReplacement() override;

        /// @brief Copy construction is disabled.
        FifoReplacement(const FifoReplacement &) = delete;

        /// @brief Copy assignment is disabled.
        FifoReplacement &operator=(const FifoReplacement &) = delete;
        /// @brief Move-constructs policy state.
        FifoReplacement(FifoReplacement &&) noexcept;

        /// @brief Move-assigns policy state.
        FifoReplacement &operator=(FifoReplacement &&) noexcept;

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
