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

#define BUF_SIZE 4096

void set_bi() {
    bi_decl(bi_program_name("Wisdom Gateway"));
    bi_decl(bi_program_description("Wisdom gateway software"))
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf
	
	// Start modem core
	while (!tud_cdc_connected()) { sleep_ms(100); };
	multicore_launch_core1(modem_core_main);

	
	//_modem_uart_init();
	//_modem_gpio_init();

	//while (!tud_cdc_connected()) { sleep_ms(100); };
	//printf("Starting modem...\n");

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
