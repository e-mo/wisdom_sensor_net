#include <stdio.h>

#include "pico/time.h"

#include "modem.h"

#define UART_PORT uart0
#define UART_BAUD 115200
#define UART_PIN_TX 0
#define UART_PIN_RX 1

#define MODEM_PIN_PWR 14
#define MODEM_APN "iot.1nce.net"

#define ever ;;

void modem_core_main(void) {
	
	printf("Starting modem... ");

	Modem *modem = modem_start(
			MODEM_APN,
			UART_PORT,
			UART_PIN_TX,
			UART_PIN_RX,
			MODEM_PIN_PWR
	);

	if (modem) printf("success!\n");

	for(ever) {
		sleep_ms(2000);
	}
}
