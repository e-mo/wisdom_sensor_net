cmake_minimum_required(VERSION 3.13)

include(radio_import.cmake)

project(${target} C CXX ASM)

add_library(${target} ${sources})

target_link_libraries(${target} ${libraries})

target_compile_definitions(${target} PUBLIC ${definitions})

# Add the standard include files to the build
target_include_directories(${target} PUBLIC ${includes})
