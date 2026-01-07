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

# ==============================================================================
# Arduino / Spresense CLI Integration
# ==============================================================================

# Variables (can be overridden via -DVAR=VALUE or cmake-gui)
set(ARDUINO_CLI "arduino-cli" CACHE STRING "Path to arduino-cli executable")
set(SPRESENSE_FQBN "SPRESENSE:spresense:spresense" CACHE STRING "FQBN for Spresense")
set(ARDUINO_FQBN "arduino:avr:uno" CACHE STRING "FQBN for Arduino Uno")
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

# Target: arduino (compile for arduino)
add_custom_target(arduino
    COMMAND ${CMAKE_COMMAND} -E echo "Switching to Arduino FQBN..."
    COMMAND ${ARDUINO_CLI} compile --fqbn ${ARDUINO_FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling for Arduino..."
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

# Target: upload_arduino
add_custom_target(upload_arduino
    COMMAND ${ARDUINO_CLI} compile --fqbn ${ARDUINO_FQBN} ${SKETCH}
    COMMAND ${ARDUINO_CLI} upload -p ${PORT} --fqbn ${ARDUINO_FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building and uploading for Arduino..."
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
    COMMAND ${CMAKE_COMMAND} -E echo "  arduino          - Compile for Arduino"
    COMMAND ${CMAKE_COMMAND} -E echo "  upload           - Upload using current settings"
    COMMAND ${CMAKE_COMMAND} -E echo "  upload_spresense - Compile and upload for Spresense"
    COMMAND ${CMAKE_COMMAND} -E echo "  upload_arduino   - Compile and upload for Arduino"
    COMMAND ${CMAKE_COMMAND} -E echo "  clean_arduino_cache - Clean arduino-cli cache"
    COMMENT "Displaying help..."
)
