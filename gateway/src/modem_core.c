#include <stdio.h>
#include <string.h>

#include "pico/time.h"

#include "modem.h"

#define UART_PORT uart0
#define UART_BAUD 115200
#define UART_PIN_TX 0
#define UART_PIN_RX 1

#define MODEM_PIN_PWR 14
#define MODEM_APN "iot.1nce.net"
#define SERVER_URL "34.27.138.203"
#define SERVER_PORT 8086

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
	
	if (modem_cn_activate(modem, true)) printf("Network activated\n");

	if (modem_ssl_enable(modem, false)) printf("SSL disabled\n");

	if (modem_tcp_open(modem, strlen(SERVER_URL), SERVER_URL, SERVER_PORT)) {
		printf("TCP connection opened\n");
	}

	if (modem_tcp_close(modem)) printf("TCP connection closed\n");

	if (modem_cn_activate(modem, false)) printf("Network deactivated\n");

	for(ever) {
		sleep_ms(2000);
	}
}
