#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "hardware/uart.h"
#include "tusb.h"

#include "modem.h"
#include "command_buffer.h"

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

#define BUF_SIZE 4096

void set_bi() {
    bi_decl(bi_program_name("Wisdom Gateway"));
    bi_decl(bi_program_description("Wisdom gateway software"))
}

void _modem_uart_init() {
	uart_init(UART_PORT, UART_BAUD); 
	gpio_set_function(UART_PIN_TX, GPIO_FUNC_UART);
	gpio_set_function(UART_PIN_RX, GPIO_FUNC_UART);
	uart_set_hw_flow(UART_PORT, false, false);
}

void _modem_gpio_init() {
	gpio_init(MODEM_PIN_PWR);
	gpio_set_dir(MODEM_PIN_PWR, GPIO_OUT);
	gpio_put(MODEM_PIN_PWR, 0);
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf
	
	// Start modem core
	multicore_launch_core1(modem_core_main);

	
	//_modem_uart_init();
	//_modem_gpio_init();

	//while (!tud_cdc_connected()) { sleep_ms(100); };
	//printf("Starting modem...\n");
	//Modem *modem = modem_start(
	//		MODEM_APN,
	//		UART_PORT,
	//		UART_PIN_TX,
	//		UART_PIN_RX,
	//		MODEM_PIN_PWR
	//);
	//printf("Modem OK!\n");	

	//char buf[BUF_SIZE];
	//bool connected = false;
	//printf("Searching for network...\n");
	//while (!connected) {
	//    connected = modem_at_send(
	//			modem,
	//			"AT+COPS?",
	//			"OK",
	//			buf,
	//			BUF_SIZE,
	//			500
	//	);
	//	if (!connected) sleep_ms(5000);
	//	if (!strstr(buf, ",")) connected = false;
	//}
	//printf("%s\n", buf);
	//printf("Network OK!\n");	

	//// Enable network connection
	//
	//modem_at_send(
	//		modem, 
	//		"AT+CNCFG=0,1,\"\"",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		10000
	//);
	//printf("%s\n", buf);

	//modem_at_send(
	//		modem,
	//		"AT+CNACT=0,1",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		20000
	//);
	//printf("%s\n", buf);

	//modem_at_send(
	//		modem,
	//		"AT+CASSLCFG=0,\"SSL\",0",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		500
	//);
	//printf("%s\n", buf);

	//modem_at_send(
	//		modem,
	//		"AT+CAOPEN=0,0,\"TCP\",\"34.27.138.203\",8086",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		3000
	//);
	//printf("%s\n", buf);

	//modem_at_send(
	//		modem,
	//		"AT+CASEND=0,4",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		500
	//);
	//printf("%s\n", buf);
	//uart_puts(uart0, "meow\r");

	//modem_at_send(
	//		modem,
	//		"AT+CACLOSE=0",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		1000
	//);
	//printf("%s\n", buf);

	//// Disable network connection
	//modem_at_send(
	//		modem,
	//		"AT+CNACT=0,0",
	//		"OK",
	//		buf,
	//		BUF_SIZE,
	//		200
	//);
	//printf("%s\n", buf);

	for(ever) {
		sleep_ms(2000);
	}
    
    return 0;
}
