#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/uart.h"
#include "tusb.h"
#include "rudp.h"

int main() {
    stdio_init_all(); // To be able to use printf


	Rfm69 *rfm = rfm69_create();
	Rfm69Config config = {
		.spi      = spi0,
		.pin_miso = RFM69_PIN_MISO,
		.pin_cs   = RFM69_PIN_CS,
		.pin_sck  = RFM69_PIN_SCK,
		.pin_mosi = RFM69_PIN_MOSI,
	    .pin_rst  = RFM69_PIN_RST
	};

	bool success = rfm69_init(rfm, &config);
	
	for(int i = 1;; i++) {
		// Wait for USB serial connection
		while (!tud_cdc_connected()) { sleep_ms(100); };

		printf("%i: %i\n", i, success);
		sleep_ms(1000);
	}
    
    return 0;
}
