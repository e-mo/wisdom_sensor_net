#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

void set_bi() {
	// ADD BI HERE
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf
					  //
	gpio_set_function(17, GPIO_FUNC_UART);
	gpio_set_function(18, GPIO_FUNC_UART);
	uart_init(uart0, 115200);
					  
	// Wait until usb serial is connected
	while (!tud_cdc_connected()) { sleep_ms(10); };

	uint8_t buf[5];
	for (;;) {

		if (uart_is_readable(uart0)) {
			printf("%c", uart_getc(uart0));
		}
			
	}
    
    return 0;
}
