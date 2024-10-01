set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)
set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")

# RTC
message("wisdom_init: initializing pcf8523 driver (RTC)")
add_subdirectory(${WISDOM_PROJECT_PATH}/drivers/pcf8523_rp2040 drivers/pcf8523_rp2040)

# Hibernation
message("wisdom_init: initializing hibernation routines")
add_subdirectory(${WISDOM_PROJECT_PATH}/libs/hibernate libs/hibernate)

# Load local config
message("wisdom_init: loading local radio_config.cmake file")
