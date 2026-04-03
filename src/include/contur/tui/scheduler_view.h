/// @file scheduler_view.h
/// @brief Scheduler panel view contracts.

#pragma once

#include "contur/core/error.h"
#include "contur/core/types.h"

#include "contur/tui/tui_models.h"

namespace contur {

    /// @brief View model for scheduler panel rendering.
    struct TuiSchedulerViewModel
    {
        /// @brief Tick represented by the panel.
        Tick currentTick = 0;

        /// @brief Scheduler snapshot payload.
        TuiSchedulerSnapshot scheduler;
    };

    /// @brief Scheduler panel renderer contract.
    class ISchedulerView
    {
        public:
        /// @brief Virtual destructor for interface-safe polymorphic cleanup.
        virtual ~ISchedulerView() = default;

        /// @brief Renders scheduler panel using immutable view model.
        /// @param model Scheduler panel view model.
        /// @return Ok on success or backend-specific error.
        [[nodiscard]] virtual Result<void> render(const TuiSchedulerViewModel &model) = 0;
    };

} // namespace contur
