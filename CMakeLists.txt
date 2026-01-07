cmake_minimum_required(VERSION 3.14)
project(SpresenseCycleComputer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include source and mock directories
# This ensures that when src/ files include <Arduino.h>, they find the mock version
include_directories(
    src
    tests/host/mocks
)

# Recursively find all source files in src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create an object library for the code in src/
# We use an OBJECT library so we don't need a main() function for compilation check
add_library(spresense_core OBJECT ${SOURCES})

# Add the tests directory
# verify if tests/host directory exists before adding it
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/CMakeLists.txt")
    add_subdirectory(tests/host)
endif()
