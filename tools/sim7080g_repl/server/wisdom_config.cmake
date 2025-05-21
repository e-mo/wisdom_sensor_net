# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "sim7080g_repl")

# Source files
list(APPEND sources
	src/repl.c	
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	circle_buffer_lib
	sim7080g_rp2x
	hardware_uart
)

list(APPEND definitions
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 1)
