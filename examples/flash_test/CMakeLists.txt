# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "~/pico/pico-sdk")

#set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.0.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

include(wisdom_import.cmake)
include(wisdom_config.cmake)

# target var declared in wisdom_config.cmake
project(${target} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(${target} ${sources})

pico_set_linker_script(${target} ${CMAKE_SOURCE_DIR}/memmap_wisdom.ld)

pico_enable_stdio_uart(${target} ${stdio_uart_enable})
pico_enable_stdio_usb(${target} ${stdio_usb_enable})

# Add the standard library to the build
target_link_libraries(${target} pico_stdlib)
target_link_libraries(${target} ${libraries})

target_compile_definitions(${target} PUBLIC ${definitions})

# Add the standard include files to the build
target_include_directories(${target} PRIVATE ${includes})

pico_add_extra_outputs(${target})
