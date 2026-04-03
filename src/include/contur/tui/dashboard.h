/// @file dashboard.h
/// @brief Composite dashboard view contract.

#pragma once

#include "contur/core/error.h"

#include "contur/tui/tui_models.h"

namespace contur {

    /// @brief Composite dashboard contract for rendering full panel layout.
    class IDashboardView
    {
        public:
        /// @brief Virtual destructor for interface-safe polymorphic cleanup.
        virtual ~IDashboardView() = default;

        /// @brief Renders dashboard from immutable snapshot.
        /// @param snapshot Full TUI snapshot.
        /// @return Ok on success or backend-specific error.
        [[nodiscard]] virtual Result<void> render(const TuiSnapshot &snapshot) = 0;

        /// @brief Clears dashboard surface.
        virtual void clear() = 0;
    };

} // namespace contur
