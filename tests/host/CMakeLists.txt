cmake_minimum_required(VERSION 3.14)
project(SpresenseCycleComputerTests)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test specific sources and mocks
set(TEST_SOURCES
    mocks/MockGlobals.cpp
    mocks/MockLibs.cpp
    TripComputerTest.cpp
    InputManagerTest.cpp
    CycleComputerTest.cpp
    ModeManagerTest.cpp
)

# Define executable
# Compile sources directly to ensure UNIT_TEST definition is applied

add_executable(run_tests
    ${TEST_SOURCES}
)

# Include directories (mocks local to tests, plus src from root)
target_include_directories(run_tests PRIVATE
    mocks
    ../../src
    .
)

# Link GoogleTest
target_link_libraries(run_tests GTest::gmock_main)

# Compiler warnings
if(MSVC)
  target_compile_options(run_tests PRIVATE /W4)
else()
  target_compile_options(run_tests PRIVATE -Wall -Wextra -pedantic)
endif()

target_compile_definitions(run_tests PRIVATE UNIT_TEST)

include(GoogleTest)
gtest_discover_tests(run_tests)
