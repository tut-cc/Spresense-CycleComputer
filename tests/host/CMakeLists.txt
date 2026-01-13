cmake_minimum_required(VERSION 3.14)
project(SpresenseCycleComputerTests)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
    mocks/MockGlobals.cpp
    mocks/MockLibs.cpp
    AppTest.cpp
    InputTest.cpp
    ModeTest.cpp
    TripTest.cpp
)

add_executable(run_tests
    ${TEST_SOURCES}
)

target_include_directories(run_tests PRIVATE
    mocks
    ../../src
    .
)

target_link_libraries(run_tests GTest::gmock_main)
target_compile_options(run_tests PRIVATE -Wall -Wextra -pedantic)
target_compile_definitions(run_tests PRIVATE UNIT_TEST)

include(GoogleTest)
gtest_discover_tests(run_tests)
