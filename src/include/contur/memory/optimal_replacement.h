/// @file optimal_replacement.h
/// @brief Optimal (Belady's) page replacement policy — educational only.

#pragma once

#include <memory>
#include <vector>

#include "contur/memory/i_page_replacement.h"

namespace contur {

    /// @brief Optimal page replacement (Belady's algorithm).
    ///
    /// Evicts the page that will not be used for the longest time in the future.
    /// Requires the complete future access sequence to be known in advance.
    /// This is impractical for real systems but serves as a theoretical baseline
    /// for comparing other algorithms' performance.
    class OptimalReplacement final : public IPageReplacementPolicy
    {
        public:
        /// @brief Constructs with a known future access sequence.
        /// @param futureAccesses Ordered list of FrameIds that will be accessed.
        explicit OptimalReplacement(std::vector<FrameId> futureAccesses);

        /// @brief Destroys optimal replacement policy.
        ~OptimalReplacement() override;

        /// @brief Copy construction is disabled.
        OptimalReplacement(const OptimalReplacement &) = delete;

        /// @brief Copy assignment is disabled.
        OptimalReplacement &operator=(const OptimalReplacement &) = delete;
        /// @brief Move-constructs policy state.
        OptimalReplacement(OptimalReplacement &&) noexcept;

        /// @brief Move-assigns policy state.
        OptimalReplacement &operator=(OptimalReplacement &&) noexcept;

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
