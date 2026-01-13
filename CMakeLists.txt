cmake_minimum_required(VERSION 3.14)
project(SpresenseCycleComputer)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(MOCK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/mocks")

add_library(spresense_core INTERFACE)

target_include_directories(spresense_core INTERFACE
    src
    ${MOCK_INCLUDE_DIR}
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/host/CMakeLists.txt")
    add_subdirectory(tests/host)
endif()

# ==============================================================================
# Arduino CLI Integration
# ==============================================================================

set(ARDUINO_CLI "arduino-cli" CACHE STRING "Path to arduino-cli executable")
set(SPRESENSE_FQBN "SPRESENSE:spresense:spresense" CACHE STRING "FQBN for Spresense")
set(PORT "/dev/ttyACM0" CACHE STRING "Serial port for upload")
set(SKETCH "." CACHE STRING "Path to sketch directory")

if(NOT DEFINED FQBN)
    set(FQBN ${SPRESENSE_FQBN})
endif()

set(SPRESENSE_INDEX_URL "https://github.com/sonydevworld/spresense-arduino-compatible/releases/download/generic/package_spresense_index.json" CACHE STRING "Spresense Package Index URL")

add_custom_target(setup
    COMMAND ${ARDUINO_CLI} core update-index --additional-urls ${SPRESENSE_INDEX_URL}
    COMMAND ${ARDUINO_CLI} core install Spresense:spresense --additional-urls ${SPRESENSE_INDEX_URL}
    COMMAND ${ARDUINO_CLI} lib install "Adafruit GFX Library"
    COMMAND ${ARDUINO_CLI} lib install "Adafruit SSD1306"
    COMMAND ${ARDUINO_CLI} lib install "Adafruit BusIO"
    COMMENT "Setting up Arduino environment..."
)

set(ARDUINO_BUILD_OPTIONS "--warnings" "all" CACHE STRING "Additional options for arduino-cli compile")

add_custom_target(spresense
    COMMAND ${CMAKE_COMMAND} -E echo "Switching to Spresense FQBN..."
    COMMAND ${ARDUINO_CLI} compile --fqbn ${SPRESENSE_FQBN} ${SKETCH} --additional-urls ${SPRESENSE_INDEX_URL} ${ARDUINO_BUILD_OPTIONS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling for Spresense..."
)

add_custom_target(upload
    COMMAND ${ARDUINO_CLI} upload -p ${PORT} --fqbn ${FQBN} ${SKETCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Uploading to ${PORT} for ${FQBN}..."
)

add_custom_target(clean_arduino_cache
    COMMAND ${ARDUINO_CLI} cache clean
    COMMENT "Cleaning Arduino CLI cache..."
)

add_custom_target(refresh
    COMMAND ${ARDUINO_CLI} compile --fqbn ${SPRESENSE_FQBN} ${SKETCH} --build-path build-arduino --only-compilation-database
    COMMAND ${CMAKE_COMMAND} -E copy build-arduino/compile_commands.json ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating compile_commands.json for IDE support..."
)

add_custom_target(howto
    COMMAND ${CMAKE_COMMAND} -E echo "Custom Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  spresense        - Compile for Spresense"
    COMMAND ${CMAKE_COMMAND} -E echo "  upload           - Upload using current settings"
    COMMAND ${CMAKE_COMMAND} -E echo "  clean_arduino_cache - Clean arduino-cli cache"
    COMMAND ${CMAKE_COMMAND} -E echo "  refresh - Generate compile_commands.json for IDE"
    COMMENT "Displaying help..."
)
