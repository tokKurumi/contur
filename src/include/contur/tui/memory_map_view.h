/// @file memory_map_view.h
/// @brief Memory panel view contracts.

#pragma once

#include "contur/core/error.h"
#include "contur/core/types.h"

#include "contur/tui/tui_models.h"

namespace contur {

    /// @brief View model for memory panel rendering.
    struct TuiMemoryMapViewModel
    {
        /// @brief Tick represented by the panel.
        Tick currentTick = 0;

        /// @brief Memory snapshot payload.
        TuiMemorySnapshot memory;
    };

    /// @brief Memory map panel renderer contract.
    class IMemoryMapView
    {
        public:
        /// @brief Virtual destructor for interface-safe polymorphic cleanup.
        virtual ~IMemoryMapView() = default;

        /// @brief Renders memory panel using immutable view model.
        /// @param model Memory panel view model.
        /// @return Ok on success or backend-specific error.
        [[nodiscard]] virtual Result<void> render(const TuiMemoryMapViewModel &model) = 0;
    };

} // namespace contur
