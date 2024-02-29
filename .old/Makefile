MAKEFLAGS += --no-print-directory
.PHONY: meow node test_rx test_tx turbidity clean

meow:
	@echo " __   __  _______  _______  _     _  ______  ";
	@echo "|  |_|  ||       ||       || | _ | ||      | ";
	@echo "|       ||    ___||   _   || || || ||___   | ";
	@echo "|       ||   |___ |  | |  ||       |  __|  | ";
	@echo "|       ||    ___||  |_|  ||       | |_____| ";
	@echo "| ||_|| ||   |___ |       ||   _   |   __    ";
	@echo "|_|   |_||_______||_______||__| |__|  |__|   ";
	@echo "(no build target)"

clean:
	rm -rf $(node_root)/build
	rm -rf $(test_rx_root)/build
	rm -rf $(test_tx_root)/build
	rm -rf $(turbidity_root)/build
	@echo " _______  ___      __   __  _______  __   __          "
	@echo "|       ||   |    |  | |  ||       ||  | |  |         "
	@echo "|    ___||   |    |  | |  ||  _____||  |_|  |         "
	@echo "|   |___ |   |    |  |_|  || |_____ |       |         "
	@echo "|    ___||   |___ |       ||_____  ||       |  /\_/\  "
	@echo "|   |    |       ||       | _____| ||   _   | ( o.o ) "
	@echo "|___|    |_______||_______||_______||__| |__|  > ^ <  "
	@echo "(all target build directories removed)"

node_root = ./node
node:
	mkdir -p $(node_root)/build
	cd $(node_root)/build; cmake ..; make

# Build both tests
test: test_rx test_tx

test_rx_root = ./test/rx
test_rx: 
	mkdir -p $(test_rx_root)/build
	cd $(test_rx_root)/build; cmake ..; make

test_tx_root = ./test/tx
test_tx:
	mkdir -p $(test_tx_root)/build
	cd $(test_tx_root)/build; cmake ..; make

turbidity_root = ./turbidity
turbidity:
	mkdir -p $(turbidity_root)/build
	cd $(turbidity_root)/build; cmake ..; make
