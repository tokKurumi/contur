/// @file ftxui_renderer.h
/// @brief FTXUI-backed IRenderer implementation for off-screen and headless rendering.
///
/// FtxuiRenderer renders a TuiSnapshot to an in-memory FTXUI Screen of fixed
/// dimensions. It is intentionally non-interactive (no event loop) so that it
/// can be used in tests and in headless pipelines without a real terminal.
///
/// For the interactive TUI loop see FtxuiApp.

#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "contur/tui/i_renderer.h"

namespace contur {

    /// @brief FTXUI off-screen renderer implementing IRenderer.
    ///
    /// Renders a TuiSnapshot to an in-memory buffer using the FTXUI DOM API.
    /// Consumers can retrieve the rendered text via lastRendered().
    class FtxuiRenderer final : public IRenderer
    {
        public:
        /// @brief Constructs renderer with given terminal dimensions.
        /// @param width  Screen width  in columns (default 120).
        /// @param height Screen height in rows    (default 40).
        explicit FtxuiRenderer(int width = 120, int height = 40);

        /// @brief Destroys renderer.
        ~FtxuiRenderer() override;

        FtxuiRenderer(const FtxuiRenderer &) = delete;
        FtxuiRenderer &operator=(const FtxuiRenderer &) = delete;

        FtxuiRenderer(FtxuiRenderer &&) noexcept;
        FtxuiRenderer &operator=(FtxuiRenderer &&) noexcept;

        /// @brief Renders snapshot to the internal screen buffer.
        [[nodiscard]] Result<void> render(const TuiSnapshot &snapshot) override;

        /// @brief Clears the internal screen buffer.
        void clear() override;

        /// @brief Returns the last rendered screen as a plain string.
        [[nodiscard]] const std::string &lastRendered() const noexcept;

        /// @brief Returns whether the buffer currently holds rendered content.
        [[nodiscard]] bool hasContent() const noexcept;

        /// @brief Screen width configured at construction.
        [[nodiscard]] int width() const noexcept;

        /// @brief Screen height configured at construction.
        [[nodiscard]] int height() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
