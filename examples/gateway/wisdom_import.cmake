set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)
set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")
set(WISDOM_MODULES_PATH "${WISDOM_PROJECT_PATH}/modules")

# GATEWAY_MODULE 
message("wisdom_init: building gateway module")
add_subdirectory(${WISDOM_MODULES_PATH}/gateway gateway_module)

# Sensor Interface
message("wisdom_init: initializing radio module")
add_subdirectory(${WISDOM_PROJECT_PATH}/sensor_interface sensor_interface)

# Load local config
message("wisdom_init: loading local wisdom_config.cmake file")
include(wisdom_config.cmake)
