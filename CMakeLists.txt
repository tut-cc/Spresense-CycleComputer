cmake_minimum_required(VERSION 3.14)
project(SpresenseCycleComputer)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories are handled per-target, but we set up the mock path for the object library
# ensuring compilation on host works
set(MOCK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/mocks")

# Recursively find all source files in src/
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create an object library for the code in src/
add_library(spresense_core OBJECT ${SOURCES})

# Configure include directories for the core library
target_include_directories(spresense_core PUBLIC
    src
    ${MOCK_INCLUDE_DIR}
)

# Add the tests directory
# verify if tests/host directory exists before adding it
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/CMakeLists.txt")
    add_subdirectory(tests/host)
endif()

# ==============================================================================
# Arduino / Spresense CLI Integration
# ==============================================================================

# Variables (can be overridden via -DVAR=VALUE or cmake-gui)
set(ARDUINO_CLI "arduino-cli" CACHE STRING "Path to arduino-cli executable")
set(SPRESENSE_FQBN "SPRESENSE:spresense:spresense" CACHE STRING "FQBN for Spresense")
set(PORT "/dev/ttyACM0" CACHE STRING "Serial port for upload")
set(SKETCH "." CACHE STRING "Path to sketch directory")

# Default FQBN to Spresense if not specified
if(NOT DEFINED FQBN)
    set(FQBN ${SPRESENSE_FQBN})
endif()

# Target: compile (generic)
add_custom_target(compile
    COMMAND ${ARDUINO_CLI} compile --fqbn ${FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling sketch for ${FQBN}..."
)

# Target: spresense (compile for spresense)
add_custom_target(spresense
    COMMAND ${CMAKE_COMMAND} -E echo "Switching to Spresense FQBN..."
    COMMAND ${ARDUINO_CLI} compile --fqbn ${SPRESENSE_FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling for Spresense..."
)

# Target: upload (generic)
add_custom_target(upload
    COMMAND ${ARDUINO_CLI} upload -p ${PORT} --fqbn ${FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Uploading to ${PORT} for ${FQBN}..."
)

# Target: upload_spresense
add_custom_target(upload_spresense
    COMMAND ${ARDUINO_CLI} compile --fqbn ${SPRESENSE_FQBN} ${SKETCH}
    COMMAND ${ARDUINO_CLI} upload -p ${PORT} --fqbn ${SPRESENSE_FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building and uploading for Spresense..."
)

# Target: clean_arduino_cache
add_custom_target(clean_arduino_cache
    COMMAND ${ARDUINO_CLI} cache clean
    COMMENT "Cleaning Arduino CLI cache..."
)

# Helper to print usage
add_custom_target(arduino_help
    COMMAND ${CMAKE_COMMAND} -E echo "Custom Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  spresense        - Compile for Spresense"
    COMMAND ${CMAKE_COMMAND} -E echo "  upload           - Upload using current settings"
    COMMAND ${CMAKE_COMMAND} -E echo "  upload_spresense - Compile and upload for Spresense"
    COMMAND ${CMAKE_COMMAND} -E echo "  clean_arduino_cache - Clean arduino-cli cache"
    COMMAND ${CMAKE_COMMAND} -E echo "  format           - Format all source files using clang-format"
    COMMENT "Displaying help..."
)

# Clang-Format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCES
        "src/*.cpp" "src/*.h"
        "tests/*.cpp" "tests/*.h"
    )
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting source code..."
    )
else()
    message(STATUS "clang-format not found. 'format' target will not be available.")
endif()
