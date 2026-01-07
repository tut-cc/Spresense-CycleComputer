cmake_minimum_required(VERSION 3.14)
project(SpresenseCycleComputerTests)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(
    ../../src
    mocks
    .
)

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

# Source files
set(SOURCES
    ../../src/system/TripComputer.cpp
    ../../src/system/InputManager.cpp
    ../../src/system/CycleComputer.cpp
    ../../src/system/ModeManager.cpp
    ../../src/system/PowerManager.cpp
    ../../src/drivers/Button.cpp
    ../../src/drivers/GPSWrapper.cpp
    mocks/MockGlobals.cpp
    mocks/MockGlobals.cpp
    TripComputerTest.cpp
    InputManagerTest.cpp
    CycleComputerTest.cpp
    ModeManagerTest.cpp
)

# Define executable
add_executable(run_tests ${SOURCES})

# Link GoogleTest
target_link_libraries(run_tests GTest::gmock_main)

# Compiler warnings
if(MSVC)
  target_compile_options(run_tests PRIVATE /W4)
else()
  target_compile_options(run_tests PRIVATE -Wall -Wextra -pedantic)
endif()
