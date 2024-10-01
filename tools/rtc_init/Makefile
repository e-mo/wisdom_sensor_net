MAKEFLAGS += --no-print-directory
SHELL := /bin/bash

# Pull in target from cmake config file
target = ${shell cat wisdom_config.cmake | grep "set(target" | sed -E 's/.*"(.*)".*/\1/'}
uf2 = build/$(target).uf2

build: clean client server

client:
	@echo "Building RTC sync client"
	mkdir -p build
	cd build; cmake ..; $(MAKE) -j8

server:
	@echo "Building RTC sync server"
	gcc -o build/rtc_sync src/rtc_sync.c

load:
	sudo picotool load $(uf2) -f

sync:
	@./build/rtc_sync

clean:
	rm -rf build

.PHONY: build client server load sync clean
