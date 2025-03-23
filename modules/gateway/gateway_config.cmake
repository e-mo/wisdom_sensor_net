# gateway_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "gateway_module")

# Source files
list(APPEND sources
	src/gateway_sim7080g.c
	src/gateway_error.c
	#src/gw_core.c
	#src/gw_core_error.c
	#src/gateway_queue.c
)

list(APPEND includes
	src
)

list(APPEND libraries
	pico_stdlib
	#pico_multicore
	sim7080g_pico
	circle_buffer_lib
)

list(APPEND definitions
	GATEWAY_UART=uart0
	GATEWAY_APN="iot.1nce.net"
	GATEWAY_PIN_TX=0
	GATEWAY_PIN_RX=1
	GATEWAY_PIN_PWR=14
)
