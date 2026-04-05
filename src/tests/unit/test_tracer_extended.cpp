/// @file test_tracer_extended.cpp
/// @brief Extended unit tests for Tracer and TraceScope — RAII depth balance,
///        level filtering mid-sequence, depth after scope exit, field routing.

#include <memory>

#include <gtest/gtest.h>

#include "contur/core/clock.h"

#include "contur/tracing/buffer_sink.h"
#include "contur/tracing/null_tracer.h"
#include "contur/tracing/trace_scope.h"
#include "contur/tracing/tracer.h"

using namespace contur;

namespace {

    /// Helper: build a Tracer with a raw BufferSink pointer for inspection.
    struct TracerFixture
    {
        SimulationClock clock;
        BufferSink *sinkRaw = nullptr;
        std::unique_ptr<Tracer> tracer;

        TracerFixture()
        {
            auto sink = std::make_unique<BufferSink>();
            sinkRaw = sink.get();
            tracer = std::make_unique<Tracer>(std::move(sink), clock);
        }
    };

} // namespace

// Depth tracking
TEST(TracerExtTest, DepthIsZeroBeforeAnyScope)
{
    TracerFixture f;
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
}

TEST(TracerExtTest, DepthReturnsToZeroAfterScopeExits)
{
    TracerFixture f;
    {
        TraceScope s(*f.tracer, "K", "op");
        EXPECT_EQ(f.tracer->currentDepth(), 1u);
    }
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
}

TEST(TracerExtTest, DepthTracksMultipleNestedScopes)
{
    TracerFixture f;
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
    {
        TraceScope l0(*f.tracer, "A", "op");
        EXPECT_EQ(f.tracer->currentDepth(), 1u);
        {
            TraceScope l1(*f.tracer, "B", "op");
            EXPECT_EQ(f.tracer->currentDepth(), 2u);
            {
                TraceScope l2(*f.tracer, "C", "op");
                EXPECT_EQ(f.tracer->currentDepth(), 3u);
            }
            EXPECT_EQ(f.tracer->currentDepth(), 2u);
        }
        EXPECT_EQ(f.tracer->currentDepth(), 1u);
    }
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
}

TEST(TracerExtTest, ManualPushPopBalancesDepth)
{
    TracerFixture f;
    f.tracer->pushScope("S", "op");
    f.tracer->pushScope("S", "inner");
    EXPECT_EQ(f.tracer->currentDepth(), 2u);
    f.tracer->popScope();
    EXPECT_EQ(f.tracer->currentDepth(), 1u);
    f.tracer->popScope();
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
}

// Min level filtering
TEST(TracerExtTest, DefaultMinLevelIsDebug)
{
    TracerFixture f;
    EXPECT_EQ(f.tracer->minLevel(), TraceLevel::Debug);
}

TEST(TracerExtTest, MinLevelCanBeRaisedAfterWrite)
{
    TracerFixture f;

    // write one debug event before raising the level
    f.tracer->trace(makeTraceEvent(0, "S", "before-raise", "", 0, TraceLevel::Debug));

    f.tracer->setMinLevel(TraceLevel::Error);

    // these should be filtered
    f.tracer->trace(makeTraceEvent(1, "S", "filtered-debug", "", 0, TraceLevel::Debug));
    f.tracer->trace(makeTraceEvent(2, "S", "filtered-info", "", 0, TraceLevel::Info));
    f.tracer->trace(makeTraceEvent(3, "S", "filtered-warn", "", 0, TraceLevel::Warn));

    // this should pass
    f.tracer->trace(makeTraceEvent(4, "S", "passes-error", "", 0, TraceLevel::Error));

    auto events = f.sinkRaw->snapshot();
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].operation, "before-raise");
    EXPECT_EQ(events[1].operation, "passes-error");
}

TEST(TracerExtTest, SetMinLevelLowerUnblocksEvents)
{
    TracerFixture f;
    f.tracer->setMinLevel(TraceLevel::Error);

    f.tracer->trace(makeTraceEvent(0, "S", "blocked", "", 0, TraceLevel::Info));
    EXPECT_EQ(f.sinkRaw->size(), 0u);

    f.tracer->setMinLevel(TraceLevel::Info);
    f.tracer->trace(makeTraceEvent(1, "S", "passes", "", 0, TraceLevel::Info));
    EXPECT_EQ(f.sinkRaw->size(), 1u);
}

// Clock integration
TEST(TracerExtTest, ClockReferenceIsAccessible)
{
    TracerFixture f;
    // The tracer must expose the same clock it was constructed with.
    EXPECT_EQ(&f.tracer->clock(), &f.clock);
}

// NullTracer
TEST(TracerExtTest, NullTracerDepthIsAlwaysZero)
{
    SimulationClock clock;
    NullTracer tracer(clock);

    tracer.pushScope("X", "op");
    tracer.pushScope("X", "inner");
    EXPECT_EQ(tracer.currentDepth(), 0u);

    tracer.popScope();
    EXPECT_EQ(tracer.currentDepth(), 0u);
}

TEST(TracerExtTest, NullTracerSetMinLevelIsNoOp)
{
    SimulationClock clock;
    NullTracer tracer(clock);

    // NullTracer's setMinLevel is a no-op — capture the initial level and
    // verify it doesn't change regardless of what we set.
    const TraceLevel initial = tracer.minLevel();

    tracer.setMinLevel(TraceLevel::Debug);
    EXPECT_EQ(tracer.minLevel(), initial);

    tracer.setMinLevel(TraceLevel::Error);
    EXPECT_EQ(tracer.minLevel(), initial);
}

// TraceScope RAII
TEST(TracerExtTest, TraceScopePushesOnConstructAndPopsOnDestruct)
{
    TracerFixture f;
    ASSERT_EQ(f.tracer->currentDepth(), 0u);

    {
        TraceScope scope(*f.tracer, "Sub", "action");
        // pushScope was called → depth = 1
        EXPECT_EQ(f.tracer->currentDepth(), 1u);
    }
    // destructor ran → popScope called → depth = 0
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
}

TEST(TracerExtTest, SequentialScopesEachBalanceDepth)
{
    TracerFixture f;

    for (int i = 0; i < 4; ++i)
    {
        TraceScope s(*f.tracer, "S", "op");
        EXPECT_EQ(f.tracer->currentDepth(), 1u) << "iteration " << i;
    }
    EXPECT_EQ(f.tracer->currentDepth(), 0u);
}
