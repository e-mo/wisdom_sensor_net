meow:
	echo " __   __  _______  _______  _     _  ______  ";
	echo "|  |_|  ||       ||       || | _ | ||      | ";
	echo "|       ||    ___||   _   || || || ||___   | ";
	echo "|       ||   |___ |  | |  ||       |  __|  | ";
	echo "|       ||    ___||  |_|  ||       | |_____| ";
	echo "| ||_|| ||   |___ |       ||   _   |   __    ";
	echo "|_|   |_||_______||_______||__| |__|  |__|   ";
	echo "(no build target)"

node_build_dir = ./node/build
node:
	mkdir -p $(node_build_dir)
	cd $(node_build_dir)
	cmake ..
	make

test:
	$(MAKE) --no-print-directory test_rx
	$(MAKE) --no-print-directory test_tx

test_rx_build_dir = ./test/rx/build
test_rx: 
	mkdir -p $(test_rx_build_dir)
	cd $(test_rx_build_dir)
	cmake ..
	make

test_tx_build_dir = ./test/tx/build
test_tx: 
	mkdir -p $(test_tx_build_dir)
	cd $(test_tx_build_dir)
	cmake ..
	make

clean:
	rm -rf $(node_build_dir)
	rm -rf $(test_rx_build_dir)
	rm -rf $(test_tx_build_dir)
	echo " _______  ___      __   __  _______  __   __          "
	echo "|       ||   |    |  | |  ||       ||  | |  |         "
	echo "|    ___||   |    |  | |  ||  _____||  |_|  |         "
	echo "|   |___ |   |    |  |_|  || |_____ |       |         "
	echo "|    ___||   |___ |       ||_____  ||       |  /\_/\  "
	echo "|   |    |       ||       | _____| ||   _   | ( o.o ) "
	echo "|___|    |_______||_______||_______||__| |__|  > ^ <  "
	echo "(all build directories removed)"

.PHONY: meow node test test_rx test_tx clean
.SILENT:
.ONESHELL:
