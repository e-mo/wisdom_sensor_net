MAKEFLAGS += --no-print-directory
SHELL := /bin/bash

export PICO_SDK_PATH = ~/pico/pico-sdk
export WHALE_PATH = ~/pico/wisdom_sensor_net/whale

uf2 := $(wildcard build/*.uf2)

build: clean copy_build_files
	@mkdir -p build
	@cd build; cmake ..; $(MAKE) -j8
	@${MAKE} splash

copy_build_files:
	@mkdir -p external
	@cp ${PICO_SDK_PATH}/external/pico_sdk_import.cmake ./external
	@cp ${WHALE_PATH}/external/whale_import.cmake ./external

flash:
	sudo picotool load $(uf2) -f
	@${MAKE} splash

splash:
	@mkdir -p external
	@cp ${WHALE_PATH}/external/whale_splash.txt ./external

	@echo
	@cat external/whale_splash.txt
	@echo

info:
	@mkdir -p external
	@cp ${WHALE_PATH}/external/whale_info.txt ./external

	@${MAKE} splash
	@cat external/whale_info.txt
	@echo

clean:
	@rm -rf build external

.PHONY: build flash splash info clean
