/**
 * @file test_main.cpp
 * @brief Main entry point for capsicain unit tests
 * 
 * This file provides the main() function for Google Test.
 * All test files are automatically discovered and run.
 */

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
