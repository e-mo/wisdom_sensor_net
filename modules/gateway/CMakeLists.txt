cmake_minimum_required(VERSION 3.13)

include(gateway_import.cmake)
include(gateway_config.cmake)

add_library(${target} ${sources})

target_link_libraries(${target} ${libraries})

target_compile_definitions(${target} PUBLIC ${definitions})

target_include_directories(${target} PUBLIC ${includes})
