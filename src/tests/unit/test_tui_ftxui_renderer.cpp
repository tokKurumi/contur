/// @file test_tui_ftxui_renderer.cpp
/// @brief Unit tests for FtxuiRenderer (off-screen FTXUI rendering).

#include <gtest/gtest.h>

#include "contur/tui/ftxui_renderer.h"
#include "contur/tui/tui_models.h"

using namespace contur;

namespace {

    TuiSnapshot makeEmptySnapshot()
    {
        TuiSnapshot s;
        s.currentTick = 0;
        s.processCount = 0;
        s.sequence = 1;
        s.epoch = 0;
        return s;
    }

    TuiSnapshot makeRichSnapshot()
    {
        TuiSnapshot s;
        s.currentTick = 42;
        s.processCount = 3;
        s.sequence = 7;
        s.epoch = 3;

        s.scheduler.policyName = "RoundRobin";
        s.scheduler.readyCount = 2;
        s.scheduler.blockedCount = 1;
        s.scheduler.runningQueue = {1};
        s.scheduler.readyQueue = {2, 3};
        s.scheduler.blockedQueue = {4};

        TuiProcessSnapshot p1;
        p1.id = 1;
        p1.name = "init";
        p1.state = ProcessState::Running;
        p1.basePriority = PriorityLevel::Realtime;
        p1.effectivePriority = PriorityLevel::Realtime;
        p1.nice = 0;
        p1.cpuTime = 10;
        s.processes.push_back(p1);

        TuiProcessSnapshot p2;
        p2.id = 2;
        p2.name = "worker";
        p2.state = ProcessState::Ready;
        p2.basePriority = PriorityLevel::Normal;
        p2.effectivePriority = PriorityLevel::Normal;
        p2.nice = 5;
        p2.cpuTime = 3;
        s.processes.push_back(p2);

        TuiProcessSnapshot p3;
        p3.id = 3;
        p3.name = "daemon";
        p3.state = ProcessState::Blocked;
        p3.basePriority = PriorityLevel::Low;
        p3.effectivePriority = PriorityLevel::Low;
        p3.nice = 10;
        p3.cpuTime = 1;
        s.processes.push_back(p3);

        s.memory.totalVirtualSlots = 1024;
        s.memory.freeVirtualSlots = 900;
        s.memory.totalFrames = std::make_optional<std::size_t>(64);
        s.memory.freeFrames = std::make_optional<std::size_t>(50);

        for (std::size_t i = 0; i < 64; ++i)
        {
            if (i < 5)
            {
                s.memory.frameOwners.push_back(std::make_optional<ProcessId>(1));
            }
            else if (i < 9)
            {
                s.memory.frameOwners.push_back(std::make_optional<ProcessId>(2));
            }
            else
            {
                s.memory.frameOwners.push_back(std::nullopt);
            }
        }

        return s;
    }

} // namespace

TEST(FtxuiRendererTest, DefaultDimensionsAreSet)
{
    FtxuiRenderer r;
    EXPECT_EQ(r.width(), 120);
    EXPECT_EQ(r.height(), 40);
}

TEST(FtxuiRendererTest, CustomDimensions)
{
    FtxuiRenderer r(80, 24);
    EXPECT_EQ(r.width(), 80);
    EXPECT_EQ(r.height(), 24);
}

TEST(FtxuiRendererTest, InitiallyNoContent)
{
    FtxuiRenderer r;
    EXPECT_FALSE(r.hasContent());
    EXPECT_TRUE(r.lastRendered().empty());
}

TEST(FtxuiRendererTest, RenderEmptySnapshotReturnsOk)
{
    FtxuiRenderer r(120, 30);
    auto result = r.render(makeEmptySnapshot());
    EXPECT_TRUE(result.isOk());
}

TEST(FtxuiRendererTest, RenderProducesContent)
{
    FtxuiRenderer r(120, 30);
    ASSERT_TRUE(r.render(makeEmptySnapshot()).isOk());
    EXPECT_TRUE(r.hasContent());
    EXPECT_FALSE(r.lastRendered().empty());
}

TEST(FtxuiRendererTest, RenderContainsTickNumber)
{
    FtxuiRenderer r(140, 40);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    EXPECT_NE(r.lastRendered().find("42"), std::string::npos) << "Expected tick '42' in rendered output";
}

TEST(FtxuiRendererTest, RenderContainsPolicyName)
{
    FtxuiRenderer r(140, 40);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    EXPECT_NE(r.lastRendered().find("RoundRobin"), std::string::npos) << "Expected 'RoundRobin' in rendered output";
}

TEST(FtxuiRendererTest, RenderContainsProcessNames)
{
    FtxuiRenderer r(140, 50);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    const auto &out = r.lastRendered();
    EXPECT_NE(out.find("init"), std::string::npos);
    EXPECT_NE(out.find("worker"), std::string::npos);
    EXPECT_NE(out.find("daemon"), std::string::npos);
}

TEST(FtxuiRendererTest, RenderContainsProcessStates)
{
    FtxuiRenderer r(140, 50);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    const auto &out = r.lastRendered();
    EXPECT_NE(out.find("Running"), std::string::npos);
    EXPECT_NE(out.find("Ready"), std::string::npos);
    EXPECT_NE(out.find("Blocked"), std::string::npos);
}

TEST(FtxuiRendererTest, RenderContainsMemorySection)
{
    FtxuiRenderer r(140, 50);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    EXPECT_NE(r.lastRendered().find("MEMORY"), std::string::npos);
}

TEST(FtxuiRendererTest, RenderContainsSchedulerSection)
{
    FtxuiRenderer r(140, 50);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    EXPECT_NE(r.lastRendered().find("SCHEDULER"), std::string::npos);
}

TEST(FtxuiRendererTest, RenderContainsProcessesSection)
{
    FtxuiRenderer r(140, 50);
    auto snap = makeRichSnapshot();
    ASSERT_TRUE(r.render(snap).isOk());
    EXPECT_NE(r.lastRendered().find("PROCESSES"), std::string::npos);
}

TEST(FtxuiRendererTest, SecondRenderOverwritesFirst)
{
    FtxuiRenderer r(140, 50);
    auto snap1 = makeEmptySnapshot();
    snap1.currentTick = 1;
    ASSERT_TRUE(r.render(snap1).isOk());
    std::string first = r.lastRendered();

    auto snap2 = makeEmptySnapshot();
    snap2.currentTick = 99;
    ASSERT_TRUE(r.render(snap2).isOk());

    EXPECT_NE(r.lastRendered().find("99"), std::string::npos);
}

TEST(FtxuiRendererTest, ClearRemovesContent)
{
    FtxuiRenderer r(120, 30);
    ASSERT_TRUE(r.render(makeEmptySnapshot()).isOk());
    EXPECT_TRUE(r.hasContent());

    r.clear();
    EXPECT_FALSE(r.hasContent());
    EXPECT_TRUE(r.lastRendered().empty());
}

TEST(FtxuiRendererTest, RenderAfterClearWorks)
{
    FtxuiRenderer r(120, 30);
    ASSERT_TRUE(r.render(makeEmptySnapshot()).isOk());
    r.clear();
    ASSERT_TRUE(r.render(makeRichSnapshot()).isOk());
    EXPECT_TRUE(r.hasContent());
}

TEST(FtxuiRendererTest, MoveConstructedRendererWorks)
{
    FtxuiRenderer r1(120, 30);
    ASSERT_TRUE(r1.render(makeRichSnapshot()).isOk());
    FtxuiRenderer r2(std::move(r1));
    EXPECT_TRUE(r2.hasContent());
    EXPECT_EQ(r2.width(), 120);
}
