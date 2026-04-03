/// @file ftxui_app.h
/// @brief Full interactive TUI application built on FTXUI ScreenInteractive.
///
/// FtxuiApp owns the render loop, keyboard event dispatch, and autoplay timer.
/// It wires together an ITuiController with FTXUI to produce a responsive,
/// full-screen OS kernel visualiser.
///
/// Keyboard map:
///   Space / p      — toggle Play / Pause
///   t / n          — single manual Tick
///   ← / h          — seek history backward 1 step
///   → / l          — seek history forward  1 step
///   Shift+← / H    — seek backward 10 steps
///   Shift+→/ L     — seek forward  10 steps
///   +              — increase autoplay speed (halve interval)
///   -              — decrease autoplay speed (double interval)
///   ↑ / k          — scroll kernel logs up
///   ↓ / j          — scroll kernel logs down
///   r              — resume autoplay from latest snapshot
///   s              — stop autoplay
///   q / Escape     — quit

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "contur/tui/i_tui_controller.h"

namespace contur {

    /// @brief Configuration for FtxuiApp behaviour.
    struct FtxuiAppConfig
    {
        /// @brief Autoplay interval used when [Space] is pressed to start play.
        std::uint32_t defaultIntervalMs = 300;

        /// @brief Kernel ticks advanced per autoplay step.
        std::size_t defaultStep = 1;

        /// @brief UI frame refresh rate in milliseconds (~30 fps).
        std::uint32_t frameIntervalMs = 33;

        /// @brief Minimum autoplay interval (fastest speed).
        std::uint32_t minIntervalMs = 50;

        /// @brief Maximum autoplay interval (slowest speed).
        std::uint32_t maxIntervalMs = 2000;

        /// @brief Optional callback returning formatted kernel log lines.
        std::function<std::vector<std::string>()> logProvider;
    };

    /// @brief Full interactive TUI application using FTXUI ScreenInteractive.
    ///
    /// Call run() to start the event loop — this blocks until the user quits.
    class FtxuiApp final
    {
        public:
        /// @brief Constructs the app bound to an existing controller.
        /// @param controller Controller to drive (must outlive FtxuiApp).
        /// @param config      Optional behaviour configuration.
        explicit FtxuiApp(ITuiController &controller, FtxuiAppConfig config = {});

        ~FtxuiApp();

        FtxuiApp(const FtxuiApp &) = delete;
        FtxuiApp &operator=(const FtxuiApp &) = delete;

        FtxuiApp(FtxuiApp &&) noexcept;
        FtxuiApp &operator=(FtxuiApp &&) noexcept;

        /// @brief Starts the interactive event loop. Blocks until the user quits.
        void run();

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace contur
