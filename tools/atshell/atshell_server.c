#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "tusb.h"

#include "modem.h"

#define ever ;; 

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_PORT uart0
#define UART_BAUD 115200
#define UART_PIN_TX 0
#define UART_PIN_RX 1

#define MODEM_PIN_PWR 14
#define MODEM_APN "iot.1nce.net"

void set_bi() {
    bi_decl(bi_program_name("AT Shell Server"));
    bi_decl(bi_program_description("Server side software for sending direct AT commands over serial"))
}

void modem_uart_init() {
	uart_init(UART_PORT, UART_BAUD); 
	gpio_set_function(UART_PIN_TX, GPIO_FUNC_UART);
	gpio_set_function(UART_PIN_RX, GPIO_FUNC_UART);
	uart_set_hw_flow(UART_PORT, false, false);
}

void modem_gpio_init() {
	gpio_init(MODEM_PIN_PWR);
	gpio_set_dir(MODEM_PIN_PWR, GPIO_OUT);
	gpio_put(MODEM_PIN_PWR, 0);
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf
	
	modem_uart_init();
	modem_gpio_init();

	while (!tud_cdc_connected()) { sleep_ms(100); };
	printf("Starting modem...\n");
	Modem *modem = modem_start(
			MODEM_APN,
			UART_PORT,
			UART_PIN_TX,
			UART_PIN_RX,
			MODEM_PIN_PWR
	);
	printf("Modem OK!\n");	

	char buf[256];
	bool connected = false;
	printf("Searching for network...\n");
	while (!connected) {
	    connected = modem_at_send(
				modem,
				"AT+COPS?",
				"OK",
				buf,
				256,
				500
		);
		if (!connected) sleep_ms(5000);
	}
	printf("%s\n", buf);
	printf("Network OK!\n");	

	modem_at_send(
			modem,
			"AT+SNPDPID=4",
			"OK",
			buf,
			256,
			200
	);
	printf("%s\n", buf);

	modem_at_send(
			modem,
			"AT+SNPING4=\"8.8.8.8\",3,1,1000",
			"OK",
			buf,
			256,
			3100	
	);
	printf("%s\n", buf);

	for(ever) {
		sleep_ms(2000);
	}
    
    return 0;
}
