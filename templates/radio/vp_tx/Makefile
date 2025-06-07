MAKEFLAGS += --no-print-directory
SHELL := /bin/bash

# Set defaults if env vars are not defined
ifndef PICO_SDK_PATH
	export PICO_SDK_PATH = ~/pico/pico-sdk
endif
ifndef WHALE_PATH
	export WHALE_PATH = ~/pico/wisdom_sensor_net/whale
endif

uf2 = $(wildcard build/*.uf2)

build: clean copy_build_files
	@mkdir -p build
	@cd build; cmake ..; $(MAKE) -j8

flash:
	sudo picotool load $(uf2) -f

build_flash: build flash

copy_build_files:
	@mkdir -p external
	@cp ${PICO_SDK_PATH}/external/pico_sdk_import.cmake ./external
	@cp ${WHALE_PATH}/external/whale_import.cmake ./external


clean:
	@rm -rf build external

.PHONY: build build_flash flash info clean
