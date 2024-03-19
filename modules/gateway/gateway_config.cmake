# gateway_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "gateway_module")

# Source files
list(APPEND sources
	src/gateway.c
	src/gateway_core.c
)

list(APPEND includes
	src
)

list(APPEND libraries
	pico_stdlib
	sim7080g_pico
)

#list(APPEND definitions
#)
