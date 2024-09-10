#include <stdio.h>
#include "pico/stdlib.h"

#include "hibernate.h"
#include "pcf8523_rp2040.h"

int main() {
	stdio_init_all();

	// Setup RTC to send interrupt soon

	// Save old clocks
	uint clock0_orig = 0;
	uint clock1_orig = 0;

	// Configure clocks to go dormant
	hibernate_run_from_dormant_source(DORMANT_SOURCE_XOSC);

	// Go dormant until falling edge of active low signal
	hibernate_goto_dormant_until_pin(2, false, false);

	// Recover clocks after hibernation
	hibernate_recover_clocks(clock0_orig, clock1_orig);

	// Attempt to print after waking up (proving USB serial is OK)
	for (;;) {

		printf("hello!\n");
		sleep_ms(1000);

	}
}
