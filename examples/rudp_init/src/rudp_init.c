// rudp_init.c
// Example of Rfm69 RUDP initialization
// Also great for testing if your radio is connected properly
//
// Prints success=true to serial every second if init was successful
// Prints success=false otherwise

#include <stdio.h>

#include "pico/stdlib.h"

#include "tusb.h"
#include "rfm69_pico.h"

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

	success = true;	
LOOP_BEGIN: 

	for(int i = 1;; i++) {
		// Wait for USB serial connection
		while (!tud_cdc_connected()) { sleep_ms(100); };

		// Print out how many times we have looped along with success variable
		printf("%i:\tsuccess=%s\n", i, success ? "true" : "false");
		// If success != true, also print error message
		if (!success) printf("\t%s\n", error);

		sleep_ms(1000);
	}
    
    return 0;
}
