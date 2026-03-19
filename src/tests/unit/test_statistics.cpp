/// @file test_statistics.cpp
/// @brief Unit tests for scheduling statistics.

#include <gtest/gtest.h>

#include "contur/scheduling/statistics.h"

using namespace contur;

TEST(StatisticsTest, StoresFirstBurstAsPrediction)
{
    Statistics stats(0.5);
    stats.recordBurst(1, 10);
    EXPECT_TRUE(stats.hasPrediction(1));
    EXPECT_EQ(stats.predictedBurst(1), 10u);
}

TEST(StatisticsTest, UpdatesPredictionUsingEwma)
{
    Statistics stats(0.5);
    stats.recordBurst(1, 10);
    stats.recordBurst(1, 6);
    EXPECT_EQ(stats.predictedBurst(1), 8u);
}

TEST(StatisticsTest, ClearRemovesPrediction)
{
    Statistics stats;
    stats.recordBurst(1, 7);
    stats.clear(1);
    EXPECT_FALSE(stats.hasPrediction(1));
    EXPECT_EQ(stats.predictedBurst(1), 0u);
}
