MAKEFLAGS += --no-print-directory
uf2 = build/gateway.uf2

default:
	@echo "Makefile: no default target"

build:
	mkdir -p build
	cd build; cmake ..; $(MAKE)

load:
	sudo picotool load $(uf2) -f

clean:
	rm -rf build

.PHONY: build load clean
