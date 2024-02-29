#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/multicore.h"

#include "modem.h"
#include "sensors.h"

#define UART_PORT uart0
#define UART_BAUD 115200
#define UART_PIN_TX 0
#define UART_PIN_RX 1

#define MODEM_PIN_PWR 14
#define MODEM_APN "iot.1nce.net"
#define SERVER_URL "34.27.138.203"
#define SERVER_PORT 8086

#define ever ;;


static uint8_t DATA_BUFFER[1024] = {0};

void modem_test(void);

void teros_11(uint32_t next) {
	uint8_t len = (next >> 16) & 0xFF;
	printf("Teros 11\n");

	printf("len: %u\n", len);

	char buf[1024] = {0};
	for (int i = 0, j = 2; i < len; i++, j++) {
		if (j == 4) {
			next = multicore_fifo_pop_blocking();
			j = 0;
		}

		buf[i] = (next >> (8 * (3 - j))) & 0xFF;

	}

	printf("%.*s\n", len, buf);

}

void fifo_irq_handler(void) {

	uint32_t next = multicore_fifo_pop_blocking();	
	uint8_t type = (next >> 24) & 0xFF;

	switch (type) {
	case TEROS_11:
		teros_11(next);
		break;
	}

}

void modem_core_main(void) {
	multicore_fifo_clear_irq();
	irq_set_exclusive_handler(SIO_IRQ_PROC1, fifo_irq_handler);

	//irq_set_enabled(SIO_IRQ_PROC1, true);

	for(ever) {
		tight_loop_contents();
	}
	
}

void modem_test(void) {
	printf("Starting modem... ");

	Modem *modem = modem_start(
			MODEM_APN,
			UART_PORT,
			UART_PIN_TX,
			UART_PIN_RX,
			MODEM_PIN_PWR
	);

	if (modem) printf("success!\n");
	else {
		printf("fail\n");
		return;
	}
	
	if (modem_cn_activate(modem, true)) printf("Network activated\n");

	if (modem_ssl_enable(modem, false)) printf("SSL disabled\n");

	if (modem_tcp_open(modem, strlen(SERVER_URL), SERVER_URL, SERVER_PORT)) {
		printf("TCP connection opened\n");
	}

	uint8_t *msg = "PING";
	if (modem_tcp_send(modem, strlen(msg), msg)) printf("Data sent!\n");

	if (modem_tcp_recv_ready_within_us(modem, 1000 * 1000 * 10)) {
		uint8_t dst[100] = {0};
		size_t received = modem_tcp_recv(modem, 100, dst);
		printf("received: %u\n", received);
		printf("%.*s\n", received, dst);
	}

	if (modem_tcp_close(modem)) printf("TCP connection closed\n");

	if (modem_cn_activate(modem, false)) printf("Network deactivated\n");

	if (modem_power_down(modem)) printf("Modem powered down\n");
}
