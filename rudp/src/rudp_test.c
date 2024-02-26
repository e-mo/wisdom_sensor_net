#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "hardware/uart.h"
#include "tusb.h"
#include "rudp.h"

int main() {
    stdio_init_all(); // To be able to use printf
	

	for(int i = 1;; i++) {
		// Wait for USB serial connection
		while (!tud_cdc_connected()) { sleep_ms(100); };

		printf("%i: empty loop...\n", i);
		sleep_ms(1000);
	}
    
    return 0;
}
