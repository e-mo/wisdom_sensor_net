// template_main.c

#include <stdio.h>

#include "pico/stdlib.h"

#include "tusb.h"

int main() {
    stdio_init_all(); // To be able to use printf

	int i = 1;
	for(;;) {
		// Wait for USB serial connection
		while (!tud_cdc_connected()) { sleep_ms(100); };

		printf("%i: Hello, world!", i);

		i++;
		sleep_ms(1000);
	}
    
    return 0;
}
