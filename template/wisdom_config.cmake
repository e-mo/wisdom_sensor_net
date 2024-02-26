# wisdom_config.cmake
# Maintained by:
#	Evan Morse
#   emorse8686@gmail.com

set(target "build_target")

# Source files
set(sources

)

# Include directories (for .h files)
set(includes

)

# These can be left alone for the most part
set(libraries
		pico_stdlib
		hardware_spi
		hardware_uart
		pico_multicore
		pico_rand
)

# HIGH_POWER def should be left
# unless RFM69 radio is not used
set(definitions
		RFM69_HIGH_POWER
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 1)
