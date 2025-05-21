set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)
set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")

# HAL
add_subdirectory(${WISDOM_PROJECT_PATH}/wisdom_hal wisdom_hal)
