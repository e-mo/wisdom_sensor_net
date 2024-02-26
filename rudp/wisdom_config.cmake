# This should match target in local Makefile
set(target "rudp_test")

set(sources
		src/rudp_test.c
		src/rudp.c
)

set(includes
		../rfm69
)

set(libraries
		pico_stdlib
		hardware_spi
		hardware_uart
		pico_multicore
		pico_rand
)

set(definitions
		RFM69_HIGH_POWER
)

# Only one of these should be enabled
set(stdio_uart_enable 0)
set(stdio_usb_enable 1)
