#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "sim7080g_pico.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

#define BUF_SIZE 4096
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  21
#define PIN_IRQ_0  21
#define PIN_IRQ_1  21

int test_gateway() {
	return 1;
//    stdio_init_all(); // To be able to use printf
//	
//	bool success = false;
//	char *error_str = "ok";
//					  
//	Modem *modem = modem_start(
//			MODEM_APN,
//			UART_PORT,
//			UART_PIN_TX,
//			UART_PIN_RX,
//			MODEM_PIN_PWR
//	);
//
//	if (modem == NULL) {
//		error_str = "modem_start returned NULL";
//		goto LOOP_BEGIN;
//	}
//
//
//	success = modem_cn_activate(modem, true)) printf("Network activated\n");
//
//	success = true;
//LOOP_BEGIN:
//	if (!success)
//		for (;;)
//			printf("Error: %s\n", error_str); 
//			sleep_ms(3000);
//		}
//
//	for(;;) {
//
//		printf("Modem init success!\n"); 
//		sleep_ms(3000);
//	}
//
//    
//    return 0;
}
