set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

# Sensor Interfac3
message("wisdom_init: initializing radio module")
add_subdirectory(${WISDOM_PROJECT_PATH}/sensor_interface sensor_interface)

# Load local config
message("wisdom_init: loading local wisdom_config.cmake file")
include(wisdom_config.cmake)
