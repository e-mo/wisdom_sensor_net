cmake_minimum_required(VERSION 3.13)

set(target circle_buffer)
project(${target} C CXX ASM)

add_library(${target}
	src/cbuffer.c
)

target_include_directories(${target} PUBLIC 
	src
)
