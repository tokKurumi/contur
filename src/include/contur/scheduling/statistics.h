/// @file statistics.h
/// @brief Scheduling statistics and CPU burst prediction.

#pragma once

#include <memory>

#include "contur/core/types.h"

namespace contur {

    /// @brief Tracks per-process burst history and EWMA burst prediction.
    class Statistics
    {
        public:
        explicit Statistics(double alpha = 0.5);
        ~Statistics();

        Statistics(const Statistics &) = delete;
        Statistics &operator=(const Statistics &) = delete;
        Statistics(Statistics &&) noexcept;
        Statistics &operator=(Statistics &&) noexcept;

        /// @brief Records an observed CPU burst for the process.
        void recordBurst(ProcessId pid, Tick burst);

        /// @brief Returns the current predicted burst (0 if unknown).
        [[nodiscard]] Tick predictedBurst(ProcessId pid) const noexcept;

        /// @brief Returns true if the process already has a prediction.
        [[nodiscard]] bool hasPrediction(ProcessId pid) const noexcept;

        /// @brief Clears statistics for one process.
        void clear(ProcessId pid);

        /// @brief Clears statistics for all processes.
        void reset();

        /// @brief Returns EWMA alpha in range (0, 1].
        [[nodiscard]] double alpha() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
