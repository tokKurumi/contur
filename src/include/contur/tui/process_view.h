/// @file process_view.h
/// @brief Process panel view contracts.

#pragma once

#include <vector>

#include "contur/core/error.h"
#include "contur/core/types.h"

#include "contur/tui/tui_models.h"

namespace contur {

    /// @brief View model for process panel rendering.
    struct TuiProcessViewModel
    {
        /// @brief Tick represented by the panel.
        Tick currentTick = 0;

        /// @brief Process rows.
        std::vector<TuiProcessSnapshot> processes;
    };

    /// @brief Process panel renderer contract.
    class IProcessView
    {
        public:
        /// @brief Virtual destructor for interface-safe polymorphic cleanup.
        virtual ~IProcessView() = default;

        /// @brief Renders process panel using immutable view model.
        /// @param model Process panel view model.
        /// @return Ok on success or backend-specific error.
        [[nodiscard]] virtual Result<void> render(const TuiProcessViewModel &model) = 0;
    };

} // namespace contur
