# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "scheduler_example")

# Source files
list(APPEND sources
	src/scheduler_example.c	
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	scheduler_module
	radio_module
)

list(APPEND definitions

)

# disable both if hibernating
set(stdio_uart_enable 0)
set(stdio_usb_enable 0)
