MAKEFLAGS += --no-print-directory
SHELL := /bin/bash

build: clean bin

bin: clean client server
	@echo "Building repl client/server"

client:
	mkdir -p client/build
	cd client/build; cmake ..; $(MAKE) -j8

server:
	mkdir -p server/build
	cd server/build; cmake ..; $(MAKE) -j8

load:
	cd server; $(MAKE) load

clean:
	rm -rf client/build
	rm -rf server/build

.PHONY: build bin client server load clean
