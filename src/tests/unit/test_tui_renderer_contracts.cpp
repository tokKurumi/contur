/// @file test_tui_renderer_contracts.cpp
/// @brief Compile/runtime contract checks for TUI renderer and panel interfaces.

#include <optional>

#include <gtest/gtest.h>

#include "contur/tui/dashboard.h"
#include "contur/tui/i_renderer.h"
#include "contur/tui/memory_map_view.h"
#include "contur/tui/process_view.h"
#include "contur/tui/scheduler_view.h"

using namespace contur;

namespace {

    class FakeRenderer final : public IRenderer
    {
        public:
        [[nodiscard]] Result<void> render(const TuiSnapshot &snapshot) override
        {
            lastTick_ = snapshot.currentTick;
            return Result<void>::ok();
        }

        void clear() override
        {
            cleared_ = true;
        }

        [[nodiscard]] Tick lastTick() const noexcept
        {
            return lastTick_;
        }

        [[nodiscard]] bool cleared() const noexcept
        {
            return cleared_;
        }

        private:
        Tick lastTick_ = 0;
        bool cleared_ = false;
    };

    class FakeProcessView final : public IProcessView
    {
        public:
        [[nodiscard]] Result<void> render(const TuiProcessViewModel &model) override
        {
            renderedCount_ = model.processes.size();
            return Result<void>::ok();
        }

        [[nodiscard]] std::size_t renderedCount() const noexcept
        {
            return renderedCount_;
        }

        private:
        std::size_t renderedCount_ = 0;
    };

    class FakeSchedulerView final : public ISchedulerView
    {
        public:
        [[nodiscard]] Result<void> render(const TuiSchedulerViewModel &model) override
        {
            readyCount_ = model.scheduler.readyCount;
            return Result<void>::ok();
        }

        [[nodiscard]] std::size_t readyCount() const noexcept
        {
            return readyCount_;
        }

        private:
        std::size_t readyCount_ = 0;
    };

    class FakeMemoryMapView final : public IMemoryMapView
    {
        public:
        [[nodiscard]] Result<void> render(const TuiMemoryMapViewModel &model) override
        {
            totalSlots_ = model.memory.totalVirtualSlots;
            return Result<void>::ok();
        }

        [[nodiscard]] std::size_t totalSlots() const noexcept
        {
            return totalSlots_;
        }

        private:
        std::size_t totalSlots_ = 0;
    };

    class FakeDashboardView final : public IDashboardView
    {
        public:
        [[nodiscard]] Result<void> render(const TuiSnapshot &snapshot) override
        {
            tick_ = snapshot.currentTick;
            return Result<void>::ok();
        }

        void clear() override
        {
            cleared_ = true;
        }

        [[nodiscard]] Tick tick() const noexcept
        {
            return tick_;
        }

        [[nodiscard]] bool cleared() const noexcept
        {
            return cleared_;
        }

        private:
        Tick tick_ = 0;
        bool cleared_ = false;
    };

} // namespace

TEST(TuiRendererContractsTest, RendererInterfaceAcceptsSnapshot)
{
    FakeRenderer renderer;

    TuiSnapshot snapshot;
    snapshot.currentTick = 55;

    ASSERT_TRUE(renderer.render(snapshot).isOk());
    EXPECT_EQ(renderer.lastTick(), 55u);

    renderer.clear();
    EXPECT_TRUE(renderer.cleared());
}

TEST(TuiRendererContractsTest, PanelContractsAcceptViewModels)
{
    FakeProcessView processView;
    FakeSchedulerView schedulerView;
    FakeMemoryMapView memoryView;

    TuiProcessViewModel processModel;
    processModel.processes = {
        TuiProcessSnapshot{.id = 1, .name = "p1", .laneIndex = std::nullopt},
        TuiProcessSnapshot{.id = 2, .name = "p2", .laneIndex = std::nullopt},
    };

    TuiSchedulerViewModel schedulerModel;
    schedulerModel.scheduler.readyCount = 3;

    TuiMemoryMapViewModel memoryModel;
    memoryModel.memory.totalVirtualSlots = 64;

    ASSERT_TRUE(processView.render(processModel).isOk());
    ASSERT_TRUE(schedulerView.render(schedulerModel).isOk());
    ASSERT_TRUE(memoryView.render(memoryModel).isOk());

    EXPECT_EQ(processView.renderedCount(), 2u);
    EXPECT_EQ(schedulerView.readyCount(), 3u);
    EXPECT_EQ(memoryView.totalSlots(), 64u);
}

TEST(TuiRendererContractsTest, DashboardContractAcceptsSnapshot)
{
    FakeDashboardView dashboard;

    TuiSnapshot snapshot;
    snapshot.currentTick = 7;

    ASSERT_TRUE(dashboard.render(snapshot).isOk());
    EXPECT_EQ(dashboard.tick(), 7u);

    dashboard.clear();
    EXPECT_TRUE(dashboard.cleared());
}
