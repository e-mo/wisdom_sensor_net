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

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(node C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(node 
	node.c
)

pico_enable_stdio_uart(node 0)
pico_enable_stdio_usb(node 1)

# Add the standard library to the build
target_link_libraries(node
        pico_stdlib
)

# Print error messages to UART
target_compile_definitions(node PUBLIC ERROR_UART) 
# Flash error codes on LED
target_compile_definitions(node PUBLIC ERROR_LED) 
# Log error messages (unimplemented)
#target_compile_definitions(node PUBLIC ERROR_LOG) 
# Enable high power module range
target_compile_definitions(node PUBLIC RFM69_HIGH_POWER)

# Add the standard include files to the build
target_include_directories(node PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(node
)

pico_add_extra_outputs(node)

