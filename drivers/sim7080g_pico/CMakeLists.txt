cmake_minimum_required(VERSION 3.13)

set(target sim7080g_pico)
project(${target} C CXX ASM)

add_library(${target} 
	src/modem.c
	src/command_buffer.c
	src/response_parser.c
)

target_include_directories(${target} PUBLIC 
	src
)

target_link_libraries(${target} PUBLIC
	pico_stdlib
	hardware_spi
	pico_rand
)
