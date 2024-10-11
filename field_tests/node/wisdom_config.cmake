# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "gateway_init")

# Source files
list(APPEND sources
	src/gateway_main.c	
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	#gateway_module
	wisdom_sensor_interface
	radio_module
	#scheduler_module
	hardware_i2c
)

#list(APPEND definitions
#)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 0)
