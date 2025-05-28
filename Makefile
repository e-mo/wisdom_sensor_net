default:
	@echo meow to you

submodules:
	git submodule update --init --recursive
	git submodule foreach --recursive git fetch
	git submodule foreach --recursive git merge origin main

.PHONY: default submodules
