set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)
set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")

# SIM7080G
message("wisdom_init: building sim7080g_pico drivers")
add_subdirectory(${WISDOM_DRIVERS_PATH}/sim7080g_pico sim7080g_pico)

# Load local config
message("wisdom_init: loading local radio_config.cmake file")
