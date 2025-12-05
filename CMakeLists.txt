cmake_minimum_required(VERSION 3.30)
project(Titan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Main Project Setup
# This lets us use #include "OrderBook.hpp" without adding "include/" every time
include_directories(include)

# The Main App (Titan)
add_executable(Titan
        src/main.cpp
        src/OrderBook.cpp
        # src/MarketSim.cpp   <-- Uncomment this later when we add it!
)

# 2. Testing Setup (Google Test)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent Google Test from overriding your compiler settings (Windows fix)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# 3. Define the Test Executable
# This is a SEPARATE program just for running tests
add_executable(TitanTests
        tests/OrderBookTests.cpp
        src/OrderBook.cpp      # We link the engine logic here so we can test it
)

# Link Google Test to our test executable
target_link_libraries(TitanTests GTest::gtest_main)

# Register it so you can run "ctest"
include(GoogleTest)
gtest_discover_tests(TitanTests)