// rudp_init.c
// Example of Rfm69 RUDP initialization
// Also great for testing if your radio is connected properly
//
// Prints success=true to serial every second if init was successful
// Prints success=false otherwise

// source.c
// Brief description

//	Copyright (C) 2024 
//	Evan Morse
//	Amelia Vlahogiannis
//	Noelle Steil
//	Jordan Allen
//	Sam Colwell
//	Rachel Cleminson

//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.

//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.

//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "tusb.h"
#include "rfm69_pico.h"

void error_loop(char *error) {
	for (;;) {
		printf("Init error: %s\n", error);
		sleep_ms(3000);
	}
}

int main() {
    stdio_init_all(); // To be able to use printf

	bool success = false;
	char *error = "success";

	// Create rfm69 context
	rfm69_context_t *rfm = rfm69_create();
	if (rfm == NULL) {
		error = "rfm69_create returned NULL";
		goto LOOP_BEGIN;
	}

	// Pull in definitions from winsdom_config.cmake
	// spi0 is a macro defined in Pico API
	rfm69_config_t config = {
		.spi      = spi0,
		.pin_miso = RFM69_PIN_MISO,
		.pin_cs   = RFM69_PIN_CS,
		.pin_sck  = RFM69_PIN_SCK,
		.pin_mosi = RFM69_PIN_MOSI,
		.pin_rst  = RFM69_PIN_RST
	};
	// Initialize rfm69 context
	if (rfm69_init(rfm, &config) == false) {
		error = "rfm69_init failed";
		goto LOOP_BEGIN;
	}
	
	// Create rudp context
	rudp_context_t *rudp = rfm69_rudp_create();
	if (rudp == NULL) {
		error = "rfm69_rudp_create returned NULL";
		goto LOOP_BEGIN;
	}

	// Initialize rudp context
	if (rfm69_rudp_init(rudp, rfm) == false) {
		error = "rfm69_rudp_init failed";
		goto LOOP_BEGIN;
	}

	// Node address can now be set through the RUDP interface
	// Set to address 0x02
	if (rfm69_rudp_address_set(rudp, 0x02) == false) {
		error = "rfm69_rudp_address_set failed";
		goto LOOP_BEGIN;
	}

	success = true;	
LOOP_BEGIN:;

	if (!success)
		error_loop(error);
	
	// Set payload
	char *payload = "Hello, Receiver!";
	uint payload_size = strlen(payload) + 1;

	rfm69_rudp_payload_set(rudp, payload, payload_size);

	// Create pointer to trx_report
	trx_report_t *report = rfm69_rudp_report_get(rudp);

	bool tx_success = false;
	uint loop = 1;
	for(;;) {
		printf("%u\n", loop++);

		// Send payload to node address 0x02
		tx_success = rfm69_rudp_transmit(rudp, 0x02);
		printf("tx success = %s\n", tx_success ? "true" : "false");

		// trx_report printing helper function
		rfm69_rudp_report_print(report);
		printf("\n");
		
		sleep_ms(1000);
	}
    
    return 0;
}
