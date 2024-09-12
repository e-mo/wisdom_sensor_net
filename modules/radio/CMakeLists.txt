cmake_minimum_required(VERSION 3.13)

include(radio_import.cmake)

project(${target} C CXX ASM)

add_library(${target} INTERFACE)

target_sources(${target} INTERFACE ${sources})

target_link_libraries(${target} INTERFACE ${libraries})

target_compile_definitions(${target} INTERFACE ${definitions})

target_include_directories(${target} INTERFACE ${includes})
