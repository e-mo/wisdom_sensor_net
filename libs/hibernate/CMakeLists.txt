# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(target "wisdom_hibernate")
project(${target} C CXX ASM)
add_library(${target} INTERFACE)

target_sources(${target} INTERFACE src/hibernate.c)

target_include_directories(${target} INTERFACE src)

target_link_libraries(${target} INTERFACE
	pico_stdlib
	pcf8523_rp2040
	hardware_clocks
	hardware_rosc
	hardware_rtc
	hardware_i2c
)
