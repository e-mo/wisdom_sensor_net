cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(target "wisdom_sensor_interface")

project(${target} C CXX ASM)

# Add executable. Default name is the project name, version 0.1

add_library(${target} 
	# Base template
	src/wisdom_sensors.c
	src/sensor_interface.c

	# Test sensor
	src/sensors/test/test_sensor.c
)

# Add the standard include files to the build
target_include_directories(${target} PUBLIC
	src	
)

#target_link_libraries(${target} PUBLIC)
