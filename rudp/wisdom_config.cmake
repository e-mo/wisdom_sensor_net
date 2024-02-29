set(target "rudp_test")

# Source files
list(APPEND sources
	src/rudp_test.c
	src/rudp.c
)

# Include file locations
list(APPEND includes
	src
)

# pico_stdlib included by default
list(APPEND libraries
	# Pico libs
	hardware_spi
	hardware_uart
	pico_multicore
	pico_rand

	# Our libs
	rfm69_pico
)

list(APPEND definitions
	# Enables high power routines for RFM69
	RFM69_HIGH_POWER

	# RFM69 pin definitions
	RFM69_PIN_MISO=16	
	RFM69_PIN_MOSI=19
	RFM69_PIN_CS=17
	RFM69_PIN_SCK=18
	RFM69_PIN_RST=20
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 1)
