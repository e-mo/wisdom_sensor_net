#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <stdio.h>

#define UART_ID uart0
#define BAUD_RATE 1200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

char rx_buffer[32];
/*
void uart0_irq_handler() {
	int i = 0;
	while(uart_is_readable(UART_ID) && rx_buffer[i] != '\0') {
		rx_buffer[i] = uart_getc(UART_ID);
		i++;
	}
	printf("read data: ");
	printf("%s\n", rx_buffer);
}
*/
int main() {

	stdio_init_all();

    sleep_ms(100);
	printf("hello");
	uart_init(UART_ID, BAUD_RATE);
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
/*
    sleep_ms(100);
	irq_set_exclusive_handler(UART0_IRQ, uart0_irq_handler);
	irq_set_enabled(UART0_IRQ, true);
	uart_set_irq_enables(UART_ID, true, false);

    sleep_ms(100);
*/
	while(1) {
	}
	
}
