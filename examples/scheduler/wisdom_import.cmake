set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

# Scheduler module
message("wisdom_init: initializing scheduler module")
add_subdirectory(${WISDOM_PROJECT_PATH}/modules/scheduler modules/scheduler)

# Radio module
message("wisdom_init: initializing radio module")
add_subdirectory(${WISDOM_PROJECT_PATH}/modules/radio modules/radio)

# Load local config
message("wisdom_init: loading local wisdom_config.cmake file")
include(wisdom_config.cmake)
