MAKEFLAGS += --no-print-directory

target = rudp_test
uf2 = build/$(target).uf2

default:
	@echo "Makefile: no default target"

build: clean
	mkdir -p build
	cd build; cmake ../common; $(MAKE) -j8

load:
	sudo picotool load $(uf2) -f

clean:
	rm -rf build

.PHONY: build load clean
