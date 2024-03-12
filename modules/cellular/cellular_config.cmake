# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "cellular_module")

# Source files
list(APPEND sources

)

list(APPEND includes
	src
)

list(APPEND libraries
	pico_stdlib
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
