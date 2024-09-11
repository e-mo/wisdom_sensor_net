# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "pcf8523_example")

# Source files
list(APPEND sources
	src/hibernate.c	
	src/test.c
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	radio_module
	pcf8523_rp2040
	hardware_clocks
	hardware_rosc
	hardware_rtc
	hardware_i2c
)

list(APPEND definitions
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 0)
