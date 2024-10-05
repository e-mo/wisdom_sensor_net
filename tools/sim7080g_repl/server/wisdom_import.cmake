set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")
set(WISDOM_LIBS_PATH "${WISDOM_PROJECT_PATH}/libs")

# SIM7080G
message("wisdom_init: building sim7080g_pico drivers")
add_subdirectory(${WISDOM_DRIVERS_PATH}/sim7080g_pico build/sim7080g_pico)

# Circle Buffer
message("wisdom_init: loading circle_buffer lib")
add_subdirectory(${WISDOM_LIBS_PATH}/circle_buffer circle_buffer)

# Load local config
message("wisdom_init: loading local wisdom_config.cmake file")
include(wisdom_config.cmake)
