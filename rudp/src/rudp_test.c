#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/uart.h"
#include "tusb.h"
#include "rudp.h"

int main() {
    stdio_init_all(); // To be able to use printf

	bool success = false;

	rfm69_context_t *rfm = rfm69_create();
	if (rfm == NULL) goto LOOP_BEGIN;

	rfm69_config_t config = {
		.spi      = spi0,
		.pin_miso = RFM69_PIN_MISO,
		.pin_cs   = RFM69_PIN_CS,
		.pin_sck  = RFM69_PIN_SCK,
		.pin_mosi = RFM69_PIN_MOSI,
	    .pin_rst  = RFM69_PIN_RST
	};
	if (rfm69_init(rfm, &config) == false)
		goto LOOP_BEGIN;

	rudp_context_t *rudp = rfm69_rudp_create();
	if (rudp == NULL) goto LOOP_BEGIN;

	if (rfm69_rudp_init(rudp, rfm) == false)
		goto LOOP_BEGIN;

	success = true;	
LOOP_BEGIN: 

	for(int i = 1;; i++) {
		// Wait for USB serial connection
		while (!tud_cdc_connected()) { sleep_ms(100); };

		printf("%i: success=%s\n", i, success ? "true" : "false");

		sleep_ms(1000);
	}
    
    return 0;
}
