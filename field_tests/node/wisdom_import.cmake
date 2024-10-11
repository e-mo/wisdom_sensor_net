set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)
set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")
set(WISDOM_MODULES_PATH "${WISDOM_PROJECT_PATH}/modules")

# GATEWAY_MODULE 
#message("wisdom_init: building gateway module")
#add_subdirectory(${WISDOM_MODULES_PATH}/gateway gateway_module)

# Sensor Interface
message("wisdom_init: initializing sensor interface")
add_subdirectory(${WISDOM_PROJECT_PATH}/sensor_interface sensor_interface)

# Radio
message("wisdom_init: initializing radio module")
add_subdirectory(${WISDOM_PROJECT_PATH}/modules/radio modules/radio)

# Scheduler module
#message("wisdom_init: initializing scheduler module")
#add_subdirectory(${WISDOM_PROJECT_PATH}/modules/scheduler modules/scheduler)
