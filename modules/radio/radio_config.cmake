# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "radio_module")

# Source files
list(APPEND sources
	src/radio_rfm69.c
	src/radio_error.c
)

list(APPEND includes
	src
)

list(APPEND libraries
	rfm69_rp2040
)

list(APPEND definitions
	# Enables high power routines for RFM69
	RFM69_HIGH_POWER

	# RFM69 pin definitions 
	RFM69_SPI=spi0
	RFM69_PIN_MISO=16	
	RFM69_PIN_MOSI=19
	RFM69_PIN_CS=17
	RFM69_PIN_SCK=18
	RFM69_PIN_RST=20
)
