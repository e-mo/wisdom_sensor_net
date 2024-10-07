cmake_minimum_required(VERSION 3.13)
set(target "s35770_rp2x")

project(${target} C CXX ASM)
add_library(${target} INTERFACE)

target_sources(${target} INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/src/s35770_rp2x.c
)

target_include_directories(${target} INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${target} INTERFACE
	pico_stdlib
	hardware_i2c
)
