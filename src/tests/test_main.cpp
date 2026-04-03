/// @file test_main.cpp
/// @brief Shared GoogleTest entry point for unit and integration test executables.

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
