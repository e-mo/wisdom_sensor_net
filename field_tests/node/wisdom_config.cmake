# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "node_field_test")

# Source files
list(APPEND sources
	src/node_main.c	
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	rp2x_rfm69_rudp_lib
	hardware_i2c
)

list(APPEND definitions
	# Enables high power routines for RFM69H series
	RFM69_HIGH_POWER

	# RFM69 pin definitions 
	RFM69_SPI=spi0
	RFM69_PIN_MISO=16	
	RFM69_PIN_MOSI=19
	RFM69_PIN_CS=17
	RFM69_PIN_SCK=18
	RFM69_PIN_RST=20
	RFM69_PIN_DIO0=10
	RFM69_PIN_DIO1=11
	RFM69_PIN_DIO2=22
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 0)
