cmake_minimum_required(VERSION 3.13)

set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

set(WISDOM_DRIVERS_PATH "${WISDOM_PROJECT_PATH}/drivers")
set(WISDOM_LIBS_PATH "${WISDOM_PROJECT_PATH}/libs")

# Serial <-> USB
message("wisdom_init: loading rp2x serial")
add_subdirectory(${WISDOM_LIBS_PATH}/rp2x_serial libs/rp2x_serial)

project(repl_client C)
add_executable(repl_client src/repl_client.c)
target_link_libraries(repl_client rp2x_serial_lib)
target_include_directories(repl_client PRIVATE .)
