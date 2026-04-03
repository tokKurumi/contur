/// @file i_renderer.h
/// @brief Backend-agnostic renderer contract for TUI MVC view boundary.

#pragma once

#include "contur/core/error.h"

#include "contur/tui/tui_models.h"

namespace contur {

    /// @brief Renderer contract for painting a full TUI snapshot.
    class IRenderer
    {
        public:
        /// @brief Virtual destructor for interface-safe polymorphic cleanup.
        virtual ~IRenderer() = default;

        /// @brief Renders a full UI snapshot.
        /// @param snapshot Immutable UI snapshot.
        /// @return Ok on success or backend-specific error.
        [[nodiscard]] virtual Result<void> render(const TuiSnapshot &snapshot) = 0;

        /// @brief Clears backend surface/frame.
        virtual void clear() = 0;
    };

} // namespace contur
