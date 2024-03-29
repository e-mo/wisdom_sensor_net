set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

# RFM69
message("wisdom_init: building rfm69_pico library")
file(COPY ${WISDOM_PROJECT_PATH}/rfm69 DESTINATION ${CMAKE_BINARY_DIR})
add_subdirectory(${CMAKE_BINARY_DIR}/rfm69)

# FatFs_SPI
message("wisdom_init: building FatFs_SPI library")
file(COPY ${WISDOM_PROJECT_PATH}/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI 
	DESTINATION ${CMAKE_BINARY_DIR})
add_subdirectory(${CMAKE_BINARY_DIR}/FatFs_SPI)

# Load local config
message("wisdom_init: loading local wisdom_config.cmake file")
include(wisdom_config.cmake)
