set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)
set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")
set(WISDOM_MODULES_PATH "${WISDOM_PROJECT_PATH}/modules")

add_subdirectory(${WISDOM_MODULES_PATH}/gateway gateway_module)
add_subdirectory(${WISDOM_DRIVERS_PATH}/rp2x_rfm69_rudp rp2x_rfm69_rudp)
