cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "~/pico/pico-sdk")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(pb_rx C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(../../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

# Add executable. Default name is the project name, version 0.1

add_executable( 
    pb_rx
    pb_rx.c
    hw_config.c
    ../../rfm69/rfm69.c
    ../../pico-ssd1306/ssd1306.c
    ../../rudp/rudp.c
	../../common/common_config.c
)

pico_enable_stdio_uart(pb_rx 0)
pico_enable_stdio_usb(pb_rx 1)

# Add the standard library to the build
target_link_libraries(
    pb_rx
    pico_stdlib
)

# Print error messages to UART
target_compile_definitions(pb_rx PUBLIC ERROR_UART) 
# Flash error codes on LED
target_compile_definitions(pb_rx PUBLIC ERROR_LED) 
# Log error messages (unimplemented)
#target_compile_definitions(pb_rx PUBLIC ERROR_LOG) 
# Enable high power module range
target_compile_definitions(pb_rx PUBLIC RFM69_HIGH_POWER)

# Add the standard include files to the build
target_include_directories(pb_rx PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
    ../../rfm69/
    ../../rudp/
    ../../pico-ssd1306/
	../../common/
)

# Add any user requested libraries
target_link_libraries(
    pb_rx
    hardware_spi
    hardware_i2c
    hardware_dma
    pico_rand
    FatFs_SPI
)

pico_add_extra_outputs(pb_rx)
