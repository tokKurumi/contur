/// @file ftxui_app.cpp
/// @brief FtxuiApp — interactive FTXUI TUI application.

#include "contur/tui/ftxui_app.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include "contur/process/priority.h"
#include "contur/process/state.h"
#include "contur/tui/i_tui_controller.h"
#include "contur/tui/tui_commands.h"
#include "contur/tui/tui_models.h"

namespace contur {

    using namespace ftxui;

    namespace {

        constexpr std::size_t kVisibleKernelLogRows = 5;

        Color stateColorApp(ProcessState s)
        {
            switch (s)
            {
            case ProcessState::Running:
                return Color::Green;
            case ProcessState::Ready:
                return Color::Cyan;
            case ProcessState::Blocked:
                return Color::Yellow;
            case ProcessState::Suspended:
                return Color::Magenta;
            case ProcessState::Terminated:
                return Color::Red;
            default:
                return Color::White;
            }
        }

        std::string pidList(const std::vector<ProcessId> &pids, std::size_t max = 6)
        {
            if (pids.empty())
            {
                return "—";
            }
            std::string s = "[";
            std::size_t n = std::min(pids.size(), max);
            for (std::size_t i = 0; i < n; ++i)
            {
                if (i)
                {
                    s += ",";
                }
                s += std::to_string(pids[i]);
            }
            if (pids.size() > max)
            {
                s += ",…+" + std::to_string(pids.size() - max);
            }
            return s + "]";
        }

        std::string padRight(std::string s, std::size_t w)
        {
            while (s.size() < w)
            {
                s += ' ';
            }
            return s;
        }

        std::string padLeft(std::string s, std::size_t w)
        {
            while (s.size() < w)
            {
                s = ' ' + s;
            }
            return s;
        }

        Element headerBlock(const std::string &value)
        {
            return text("  " + value + "  ");
        }

        Element headerCell(const std::string &label, const std::string &value, Color valueColor, bool valueBold)
        {
            Element valueElement = text(value) | color(valueColor);
            if (valueBold)
            {
                valueElement = valueElement | bold;
            }

            return hbox({
                text("  " + label + " ") | bold,
                valueElement,
                text("  "),
            });
        }

        // Header bar
        Element buildHeaderBar(const TuiSnapshot &snap, TuiControllerState ctrlState, std::uint32_t intervalMs)
        {
            std::string stateStr;
            Color stateCol = Color::White;
            switch (ctrlState)
            {
            case TuiControllerState::Playing:
                stateStr = "▶ PLAYING";
                stateCol = Color::Green;
                break;
            case TuiControllerState::Paused:
                stateStr = "⏸ PAUSED";
                stateCol = Color::Yellow;
                break;
            case TuiControllerState::Idle:
            default:
                stateStr = "■ IDLE";
                stateCol = Color::GrayLight;
                break;
            }

            return hbox({
                       headerBlock("CONTUR2 OS Simulator") | bold | color(Color::Blue),
                       separator(),
                       headerCell("Tick:", std::to_string(snap.currentTick), Color::Cyan, true),
                       separator(),
                       headerCell(
                           "Policy:",
                           snap.scheduler.policyName.empty() ? "—" : snap.scheduler.policyName,
                           Color::White,
                           false
                       ),
                       separator(),
                       headerCell("Procs:", std::to_string(snap.processCount), Color::White, false),
                       separator(),
                       headerCell("Speed:", std::to_string(intervalMs) + "ms", Color::GrayLight, false),
                       separator(),
                       headerBlock(stateStr) | color(stateCol) | bold,
                   }) |
                   border;
        }

        // Process table
        Element buildProcessTable(const TuiSnapshot &snap)
        {
            Elements rows;
            rows.push_back(hbox({
                text(padRight(" PID", 5)) | bold,
                separator(),
                text(padRight(" Name", 16)) | bold,
                separator(),
                text(padRight(" State", 11)) | bold,
                separator(),
                text(padRight(" Priority", 12)) | bold,
                separator(),
                text(padLeft("Nice", 5)) | bold,
                separator(),
                text(padLeft("CPU", 6)) | bold,
            }));
            rows.push_back(separator());

            if (snap.processes.empty())
            {
                rows.push_back(text("  (no processes)") | dim);
            }

            for (const auto &p : snap.processes)
            {
                rows.push_back(hbox({
                    text(padRight(" " + std::to_string(p.id), 5)) | color(Color::White),
                    separator(),
                    text(padRight(" " + p.name.substr(0, 14), 16)) | color(Color::White),
                    separator(),
                    text(padRight(" " + std::string(processStateName(p.state)), 11)) | color(stateColorApp(p.state)),
                    separator(),
                    text(padRight(" " + std::string(priorityLevelName(p.effectivePriority)), 12)) | color(Color::White),
                    separator(),
                    text(padLeft(std::to_string(p.nice), 5)) | color(Color::White),
                    separator(),
                    text(padLeft(std::to_string(p.cpuTime), 6)) | color(Color::White),
                }));
            }

            return window(text("─ PROCESSES (" + std::to_string(snap.processCount) + ") "), vbox(std::move(rows))) |
                   flex;
        }

        // Scheduler panel
        Element buildSchedulerPanel(const TuiSnapshot &snap)
        {
            const auto &s = snap.scheduler;
            Elements rows;
            rows.push_back(
                hbox({text("Policy:  ") | bold, text(s.policyName.empty() ? "—" : s.policyName) | color(Color::Cyan)})
            );
            rows.push_back(separator());
            rows.push_back(hbox(
                {text("Running:  ") | bold | color(Color::Green), text(pidList(s.runningQueue)) | color(Color::Green)}
            ));
            rows.push_back(hbox(
                {text("Ready:    ") | bold | color(Color::Cyan),
                 text(pidList(s.readyQueue) + " (" + std::to_string(s.readyCount) + ")") | color(Color::Cyan)}
            ));
            rows.push_back(hbox(
                {text("Blocked:  ") | bold | color(Color::Yellow),
                 text(pidList(s.blockedQueue) + " (" + std::to_string(s.blockedCount) + ")") | color(Color::Yellow)}
            ));

            if (!s.perLaneReadyQueues.empty())
            {
                rows.push_back(separator());
                rows.push_back(text("Per-lane:") | bold | dim);
                for (std::size_t i = 0; i < s.perLaneReadyQueues.size(); ++i)
                {
                    rows.push_back(hbox(
                        {text("  Lane " + std::to_string(i) + ": ") | dim, text(pidList(s.perLaneReadyQueues[i])) | dim}
                    ));
                }
            }

            return window(text("─ SCHEDULER "), vbox(std::move(rows))) | flex;
        }

        // Memory panel
        Element buildMemoryPanel(const TuiSnapshot &snap)
        {
            const auto &m = snap.memory;
            Elements rows;

            std::size_t usedV = m.totalVirtualSlots > m.freeVirtualSlots ? m.totalVirtualSlots - m.freeVirtualSlots : 0;
            rows.push_back(hbox(
                {text("Virtual:  ") | bold,
                 text(std::to_string(usedV) + "/" + std::to_string(m.totalVirtualSlots) + " used") |
                     color(Color::White)}
            ));

            if (m.totalFrames)
            {
                std::size_t usedP =
                    *m.totalFrames > m.freeFrames.value_or(0) ? *m.totalFrames - m.freeFrames.value_or(0) : 0;
                rows.push_back(hbox(
                    {text("Physical: ") | bold,
                     text(std::to_string(usedP) + "/" + std::to_string(*m.totalFrames) + " frames") |
                         color(Color::White)}
                ));

                // Usage bar
                if (*m.totalFrames > 0)
                {
                    int barW = 20;
                    int filled = static_cast<int>(usedP * barW / *m.totalFrames);
                    std::string bar(filled, '#');
                    bar += std::string(barW - filled, '.');
                    rows.push_back(hbox(
                        {text("[") | dim,
                         text(bar) | color(filled > barW * 2 / 3 ? Color::Red : Color::Green),
                         text("]") | dim}
                    ));
                }
            }

            if (!m.frameOwners.empty())
            {
                rows.push_back(separator());
                rows.push_back(text("Frames:") | bold | dim);
                constexpr std::size_t perRow = 12;
                std::size_t idx = 0;
                while (idx < m.frameOwners.size() && idx < 48)
                {
                    Elements fr;
                    for (std::size_t j = 0; j < perRow && idx < m.frameOwners.size(); ++j, ++idx)
                    {
                        const auto &owner = m.frameOwners[idx];
                        if (owner)
                        {
                            fr.push_back(text("[" + padLeft(std::to_string(*owner), 2) + "]") | color(Color::Green));
                        }
                        else
                        {
                            fr.push_back(text("[  ]") | color(Color::GrayDark));
                        }
                    }
                    rows.push_back(hbox(std::move(fr)));
                }
                if (m.frameOwners.size() > 48)
                {
                    rows.push_back(text("  …+" + std::to_string(m.frameOwners.size() - 48) + " more") | dim);
                }
            }

            return window(text("─ MEMORY "), vbox(std::move(rows))) | flex;
        }

        // History progress bar
        Element buildHistoryBar(std::size_t cursor, std::size_t total)
        {
            constexpr int barW = 40;
            int filled = (total > 1) ? static_cast<int>(cursor * barW / (total - 1)) : barW;
            filled = std::min(filled, barW);
            std::string bar(filled, '=');
            if (filled < barW)
            {
                bar += '>';
            }
            bar += std::string(std::max(0, barW - filled - 1), ' ');

            return hbox({
                       text("  History  [") | bold,
                       text(bar) | color(Color::Cyan),
                       text("]  ") | bold,
                       text(std::to_string(cursor + 1) + " / " + std::to_string(total)) | color(Color::White),
                   }) |
                   border;
        }

        // Logs panel
        Element buildKernelLogsPanel(const std::vector<std::string> &lines, std::size_t offsetFromBottom)
        {
            Elements rows;
            if (lines.empty())
            {
                rows.push_back(text("  (no kernel logs yet)") | dim);
                return window(text("- KERNEL LOGS "), vbox(std::move(rows))) | xflex;
            }

            const std::size_t total = lines.size();
            const std::size_t maxOffset = total > kVisibleKernelLogRows ? total - kVisibleKernelLogRows : 0;
            const std::size_t offset = std::min(offsetFromBottom, maxOffset);
            const std::size_t start = total > kVisibleKernelLogRows ? (total - kVisibleKernelLogRows - offset) : 0;
            const std::size_t end = std::min(total, start + kVisibleKernelLogRows);

            for (std::size_t i = start; i < end; ++i)
            {
                rows.push_back(text(" " + lines[i]) | color(Color::GrayLight));
            }

            std::string title = "- KERNEL LOGS " + std::to_string(end) + "/" + std::to_string(total);
            if (offset > 0)
            {
                title += " [SCROLLED]";
            }

            return window(text(title), vbox(std::move(rows))) | xflex;
        }

        // Controls footer
        Element buildFooter()
        {
            return hbox({
                       headerBlock("[Space] Play/Pause") | bold | color(Color::Green),
                       separator(),
                       headerBlock("[←][→] Seek 1") | color(Color::Cyan),
                       separator(),
                       headerBlock("[H][L] Seek 10") | color(Color::Cyan),
                       separator(),
                       headerBlock("[t] Tick") | color(Color::White),
                       separator(),
                       headerBlock("[+][-] Speed") | color(Color::Yellow),
                       separator(),
                       headerBlock("[Up][Down] Logs") | color(Color::GrayLight),
                       separator(),
                       headerBlock("[r] Resume") | color(Color::Magenta),
                       separator(),
                       headerBlock("[s] Stop") | color(Color::Red),
                       separator(),
                       headerBlock("[q] Quit") | bold | color(Color::Red),
                   }) |
                   border;
        }

    } // anonymous namespace

    struct FtxuiApp::Impl
    {
        ITuiController &controller;
        FtxuiAppConfig cfg;
        std::uint32_t currentIntervalMs;
        std::size_t kernelLogOffsetFromBottom = 0;
        std::atomic<bool> running{false};

        Impl(ITuiController &ctrl, FtxuiAppConfig c)
            : controller(ctrl)
            , cfg(c)
            , currentIntervalMs(c.defaultIntervalMs)
        {}

        void togglePlay()
        {
            auto state = controller.state();
            if (state == TuiControllerState::Playing)
            {
                (void)controller.dispatch(TuiCommand{TuiCommandKind::Pause, 1, currentIntervalMs});
            }
            else
            {
                (void)controller.dispatch(
                    TuiCommand{TuiCommandKind::AutoPlayStart, cfg.defaultStep, currentIntervalMs}
                );
            }
        }

        void speedUp()
        {
            currentIntervalMs = std::max(cfg.minIntervalMs, currentIntervalMs / 2);
            if (controller.state() == TuiControllerState::Playing)
            {
                // Restart with new interval
                (void)controller.dispatch(TuiCommand{TuiCommandKind::AutoPlayStop, 1, 0});
                (void)controller.dispatch(
                    TuiCommand{TuiCommandKind::AutoPlayStart, cfg.defaultStep, currentIntervalMs}
                );
            }
        }

        void speedDown()
        {
            currentIntervalMs = std::min(cfg.maxIntervalMs, currentIntervalMs * 2);
            if (controller.state() == TuiControllerState::Playing)
            {
                (void)controller.dispatch(TuiCommand{TuiCommandKind::AutoPlayStop, 1, 0});
                (void)controller.dispatch(
                    TuiCommand{TuiCommandKind::AutoPlayStart, cfg.defaultStep, currentIntervalMs}
                );
            }
        }

        [[nodiscard]] std::vector<std::string> kernelLogs() const
        {
            if (!cfg.logProvider)
            {
                return {};
            }
            return cfg.logProvider();
        }

        void normalizeKernelLogOffset(std::size_t totalLogLines)
        {
            const std::size_t maxOffset =
                totalLogLines > kVisibleKernelLogRows ? totalLogLines - kVisibleKernelLogRows : 0;
            kernelLogOffsetFromBottom = std::min(kernelLogOffsetFromBottom, maxOffset);
        }

        void scrollKernelLogsUp()
        {
            const std::size_t totalLogLines = kernelLogs().size();
            const std::size_t maxOffset =
                totalLogLines > kVisibleKernelLogRows ? totalLogLines - kVisibleKernelLogRows : 0;
            if (kernelLogOffsetFromBottom < maxOffset)
            {
                ++kernelLogOffsetFromBottom;
            }
        }

        void scrollKernelLogsDown()
        {
            if (kernelLogOffsetFromBottom > 0)
            {
                --kernelLogOffsetFromBottom;
            }
        }

        Element buildFrame()
        {
            const auto &snap = controller.current();
            auto state = controller.state();
            std::size_t cursor = controller.historyCursor();
            std::size_t total = controller.historySize();
            auto logs = kernelLogs();
            normalizeKernelLogOffset(logs.size());

            return vbox({
                buildHeaderBar(snap, state, currentIntervalMs),
                hbox({
                    buildProcessTable(snap),
                    buildSchedulerPanel(snap),
                    buildMemoryPanel(snap),
                }) | flex,
                buildKernelLogsPanel(logs, kernelLogOffsetFromBottom),
                buildHistoryBar(cursor, total),
                buildFooter(),
            });
        }
    };

    FtxuiApp::FtxuiApp(ITuiController &controller, FtxuiAppConfig config)
        : impl_(std::make_unique<Impl>(controller, config))
    {}

    FtxuiApp::~FtxuiApp() = default;
    FtxuiApp::FtxuiApp(FtxuiApp &&) noexcept = default;
    FtxuiApp &FtxuiApp::operator=(FtxuiApp &&) noexcept = default;

    void FtxuiApp::run()
    {
        auto &imp = *impl_;
        imp.running.store(true);

        auto screen = ScreenInteractive::Fullscreen();

        // Renderer component: re-reads from controller on every frame
        auto renderer = Renderer([&] { return imp.buildFrame(); });

        // Keyboard / event handler
        auto withEvents = CatchEvent(renderer, [&](Event e) -> bool {
            if (e == Event::Character('q') || e == Event::Escape)
            {
                imp.running.store(false);
                screen.ExitLoopClosure()();
                return true;
            }
            if (e == Event::Character(' ') || e == Event::Character('p'))
            {
                imp.togglePlay();
                return true;
            }
            if (e == Event::Character('t') || e == Event::Character('n'))
            {
                (void)imp.controller.dispatch(TuiCommand{TuiCommandKind::Tick, 1, 0});
                return true;
            }
            if (e == Event::ArrowLeft || e == Event::Character('h'))
            {
                (void)imp.controller.dispatch(TuiCommand{TuiCommandKind::SeekBackward, 1, 0});
                return true;
            }
            if (e == Event::ArrowRight || e == Event::Character('l'))
            {
                (void)imp.controller.dispatch(TuiCommand{TuiCommandKind::SeekForward, 1, 0});
                return true;
            }
            if (e == Event::Character('H'))
            {
                (void)imp.controller.dispatch(TuiCommand{TuiCommandKind::SeekBackward, 10, 0});
                return true;
            }
            if (e == Event::Character('L'))
            {
                (void)imp.controller.dispatch(TuiCommand{TuiCommandKind::SeekForward, 10, 0});
                return true;
            }
            if (e == Event::Character('+') || e == Event::Character('='))
            {
                imp.speedUp();
                return true;
            }
            if (e == Event::Character('-') || e == Event::Character('_'))
            {
                imp.speedDown();
                return true;
            }
            if (e == Event::ArrowUp || e == Event::Character('k'))
            {
                imp.scrollKernelLogsUp();
                return true;
            }
            if (e == Event::ArrowDown || e == Event::Character('j'))
            {
                imp.scrollKernelLogsDown();
                return true;
            }
            if (e == Event::Character('r'))
            {
                (void)imp.controller.dispatch(
                    TuiCommand{TuiCommandKind::AutoPlayStart, imp.cfg.defaultStep, imp.currentIntervalMs}
                );
                return true;
            }
            if (e == Event::Character('s'))
            {
                (void)imp.controller.dispatch(TuiCommand{TuiCommandKind::AutoPlayStop, 1, 0});
                return true;
            }
            return false;
        });

        // Background timer thread: drives autoplay + frame refresh
        std::thread timerThread([&] {
            while (imp.running.load())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(imp.cfg.frameIntervalMs));
                (void)imp.controller.advanceAutoplay(imp.cfg.frameIntervalMs);
                screen.PostEvent(Event::Custom);
            }
        });

        screen.Loop(withEvents);

        imp.running.store(false);
        if (timerThread.joinable())
        {
            timerThread.join();
        }
    }

} // namespace contur
