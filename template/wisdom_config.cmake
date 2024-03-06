# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "target_name")

# Source files
list(APPEND sources
	src/template_main.c	
)

# Include file locations
list(APPEND includes

)

# pico_stdlib included by default
list(APPEND libraries
	#hardware_spi
	#hardware_i2c
	#hardware_uart
	#pico_multicore
	#pico_rand

	# Our libs
	#rfm69_pico
)

list(APPEND definitions
	# Enables high power routines for RFM69
	#RFM69_HIGH_POWER

	# RFM69 pin definitions
	#RFM69_PIN_MISO=16	
	#RFM69_PIN_MOSI=19
	#RFM69_PIN_CS=17
	#RFM69_PIN_SCK=18
	#RFM69_PIN_RST=20
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 1)
