cmake_minimum_required(VERSION 3.13)

set(target sim7080g_rp2x)

add_library(${target} INTERFACE)

target_sources(${target} INTERFACE
	src/sim7080g_rp2x.c
	src/cbuffer.c
)

target_include_directories(${target} INTERFACE
	src
)

target_link_libraries(${target} INTERFACE
	hardware_uart
)

