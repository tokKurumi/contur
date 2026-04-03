/// @file ftxui_renderer.cpp
/// @brief FtxuiRenderer — FTXUI off-screen IRenderer implementation.

#include "contur/tui/ftxui_renderer.h"

#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include "contur/process/priority.h"
#include "contur/process/state.h"
#include "contur/tui/tui_models.h"

namespace contur {

    namespace {

        using namespace ftxui;

        Color stateColor(ProcessState s)
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
            case ProcessState::New:
            default:
                return Color::White;
            }
        }

        std::string pidListStr(const std::vector<ProcessId> &pids, std::size_t maxShow = 8)
        {
            if (pids.empty())
            {
                return "—";
            }
            std::string out = "[";
            std::size_t shown = std::min(pids.size(), maxShow);
            for (std::size_t i = 0; i < shown; ++i)
            {
                if (i > 0)
                {
                    out += ", ";
                }
                out += std::to_string(pids[i]);
            }
            if (pids.size() > maxShow)
            {
                out += ", +" + std::to_string(pids.size() - maxShow) + " more";
            }
            out += "]";
            return out;
        }

        // Process table panel
        Element buildProcessPanel(const TuiSnapshot &snap)
        {
            Elements rows;
            // Header row
            rows.push_back(hbox({
                text(" PID  ") | bold,
                separator(),
                text(" Name              ") | bold,
                separator(),
                text(" State      ") | bold,
                separator(),
                text(" Priority    ") | bold,
                separator(),
                text(" Nice ") | bold,
                separator(),
                text(" CPU  ") | bold,
            }));
            rows.push_back(separator());

            if (snap.processes.empty())
            {
                rows.push_back(text("  (no processes)") | dim);
            }

            for (const auto &p : snap.processes)
            {
                auto stateStr = std::string(processStateName(p.state));
                // Pad to fixed width
                while (stateStr.size() < 10)
                {
                    stateStr += ' ';
                }
                auto priStr = std::string(priorityLevelName(p.effectivePriority));
                while (priStr.size() < 11)
                {
                    priStr += ' ';
                }
                auto nameStr = p.name.substr(0, 17);
                while (nameStr.size() < 17)
                {
                    nameStr += ' ';
                }
                auto pidStr = std::to_string(p.id);
                while (pidStr.size() < 4)
                {
                    pidStr = ' ' + pidStr;
                }
                auto cpuStr = std::to_string(p.cpuTime);
                while (cpuStr.size() < 4)
                {
                    cpuStr = ' ' + cpuStr;
                }
                auto niceStr = std::to_string(p.nice);
                while (niceStr.size() < 4)
                {
                    niceStr = ' ' + niceStr;
                }

                rows.push_back(hbox({
                    text(" " + pidStr + " ") | color(Color::White),
                    separator(),
                    text(" " + nameStr + " ") | color(Color::White),
                    separator(),
                    text(" " + stateStr + " ") | color(stateColor(p.state)),
                    separator(),
                    text(" " + priStr + " ") | color(Color::White),
                    separator(),
                    text(" " + niceStr + " ") | color(Color::White),
                    separator(),
                    text(" " + cpuStr + " ") | color(Color::White),
                }));
            }

            return window(text("─ PROCESSES (" + std::to_string(snap.processCount) + ") "), vbox(std::move(rows)));
        }

        // Scheduler panel
        Element buildSchedulerPanel(const TuiSnapshot &snap)
        {
            const auto &s = snap.scheduler;
            Elements rows;
            rows.push_back(
                hbox({text("Policy: ") | bold, text(s.policyName.empty() ? "—" : s.policyName) | color(Color::Cyan)})
            );
            rows.push_back(separator());
            rows.push_back(hbox(
                {text("Running:  ") | bold | color(Color::Green),
                 text(pidListStr(s.runningQueue)) | color(Color::Green)}
            ));
            rows.push_back(hbox(
                {text("Ready:    ") | bold | color(Color::Cyan),
                 text(pidListStr(s.readyQueue) + "  (" + std::to_string(s.readyCount) + ")") | color(Color::Cyan)}
            ));
            rows.push_back(hbox(
                {text("Blocked:  ") | bold | color(Color::Yellow),
                 text(pidListStr(s.blockedQueue) + "  (" + std::to_string(s.blockedCount) + ")") | color(Color::Yellow)}
            ));

            if (!s.perLaneReadyQueues.empty())
            {
                rows.push_back(separator());
                rows.push_back(text("Per-lane:") | bold | dim);
                for (std::size_t i = 0; i < s.perLaneReadyQueues.size(); ++i)
                {
                    rows.push_back(hbox(
                        {text("  Lane " + std::to_string(i) + ": ") | dim,
                         text(pidListStr(s.perLaneReadyQueues[i])) | dim}
                    ));
                }
            }

            return window(text("─ SCHEDULER "), vbox(std::move(rows)));
        }

        // Memory panel
        Element buildMemoryPanel(const TuiSnapshot &snap)
        {
            const auto &m = snap.memory;
            Elements rows;

            std::size_t usedVirt =
                m.totalVirtualSlots > m.freeVirtualSlots ? (m.totalVirtualSlots - m.freeVirtualSlots) : 0;
            rows.push_back(hbox(
                {text("Virtual:  ") | bold,
                 text(std::to_string(usedVirt) + " / " + std::to_string(m.totalVirtualSlots) + " used")}
            ));

            if (m.totalFrames.has_value())
            {
                std::size_t usedPhys =
                    *m.totalFrames > m.freeFrames.value_or(0) ? (*m.totalFrames - m.freeFrames.value_or(0)) : 0;
                rows.push_back(hbox(
                    {text("Physical: ") | bold,
                     text(std::to_string(usedPhys) + " / " + std::to_string(*m.totalFrames) + " frames used")}
                ));
            }

            if (!m.frameOwners.empty())
            {
                rows.push_back(separator());
                rows.push_back(text("Frame map:") | bold | dim);

                // Build frame map in rows of 16
                constexpr std::size_t framesPerRow = 16;
                std::size_t idx = 0;
                while (idx < m.frameOwners.size())
                {
                    Elements frameRow;
                    for (std::size_t j = 0; j < framesPerRow && idx < m.frameOwners.size(); ++j, ++idx)
                    {
                        const auto &owner = m.frameOwners[idx];
                        if (owner.has_value())
                        {
                            std::string cell = std::to_string(*owner);
                            if (cell.size() < 2)
                            {
                                cell = " " + cell;
                            }
                            frameRow.push_back(text("[" + cell + "]") | color(Color::Green));
                        }
                        else
                        {
                            frameRow.push_back(text("[  ]") | color(Color::GrayDark));
                        }
                    }
                    rows.push_back(hbox(std::move(frameRow)));
                }
            }

            return window(text("─ MEMORY "), vbox(std::move(rows)));
        }

        // Header bar
        Element buildHeader(const TuiSnapshot &snap)
        {
            return hbox({
                       text(" CONTUR2 OS Simulator ") | bold | color(Color::Blue),
                       separator(),
                       text(" Tick: ") | bold,
                       text(std::to_string(snap.currentTick)) | color(Color::Cyan),
                       separator(),
                       text(" Seq: ") | bold,
                       text(std::to_string(snap.sequence)) | color(Color::GrayLight),
                       separator(),
                       text(" Epoch: ") | bold,
                       text(std::to_string(snap.epoch)) | color(Color::GrayLight),
                   }) |
                   border;
        }

    } // anonymous namespace

    struct FtxuiRenderer::Impl
    {
        int width;
        int height;
        std::string lastOutput;
        bool hasContent = false;

        Impl(int w, int h)
            : width(w)
            , height(h)
        {}

        void renderSnapshot(const TuiSnapshot &snap)
        {
            using namespace ftxui;

            auto header = buildHeader(snap);
            auto procPanel = buildProcessPanel(snap);
            auto schedPanel = buildSchedulerPanel(snap);
            auto memPanel = buildMemoryPanel(snap);

            auto body = hbox({
                procPanel | flex,
                schedPanel | flex,
                memPanel | flex,
            });

            auto root = vbox({
                header,
                body | flex,
            });

            auto screen = Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
            Render(screen, root);
            lastOutput = screen.ToString();
            hasContent = true;
        }
    };

    FtxuiRenderer::FtxuiRenderer(int width, int height)
        : impl_(std::make_unique<Impl>(width, height))
    {}

    FtxuiRenderer::~FtxuiRenderer() = default;
    FtxuiRenderer::FtxuiRenderer(FtxuiRenderer &&) noexcept = default;
    FtxuiRenderer &FtxuiRenderer::operator=(FtxuiRenderer &&) noexcept = default;

    Result<void> FtxuiRenderer::render(const TuiSnapshot &snapshot)
    {
        if (!impl_)
        {
            return Result<void>::error(ErrorCode::InvalidState);
        }
        impl_->renderSnapshot(snapshot);
        return Result<void>::ok();
    }

    void FtxuiRenderer::clear()
    {
        if (impl_)
        {
            impl_->lastOutput.clear();
            impl_->hasContent = false;
        }
    }

    const std::string &FtxuiRenderer::lastRendered() const noexcept
    {
        static const std::string kEmpty;
        return impl_ ? impl_->lastOutput : kEmpty;
    }

    bool FtxuiRenderer::hasContent() const noexcept
    {
        return impl_ && impl_->hasContent;
    }

    int FtxuiRenderer::width() const noexcept
    {
        return impl_ ? impl_->width : 0;
    }

    int FtxuiRenderer::height() const noexcept
    {
        return impl_ ? impl_->height : 0;
    }

} // namespace contur
