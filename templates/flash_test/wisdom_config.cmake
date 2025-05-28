# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com
set(target "flase_test")

# Source files
list(APPEND sources
	src/flash_test.c	
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	wisdom_hal
	hardware_flash
	hardware_sync
)

list(APPEND definitions
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 1)
