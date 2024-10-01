# wisdom_config.cmake
# Maintainer:
#	Evan Morse
#   emorse8686@gmail.com

# DO NOT MODIFY THE FORMATTING OF THIS LINE
# Only change the target name
set(target "scheduler_module")

# Source files
list(APPEND sources
	src/scheduler_module.c	
)

list(APPEND includes
	src
)

list(APPEND libraries
	pcf8523_rp2040
	wisdom_hibernate
	hardware_i2c
)

list(APPEND definitions
)
