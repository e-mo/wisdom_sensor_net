#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

#include "cbuffer.h"

#define UART_INST (uart0)
#define UART_BAUD (115200)
#define PIN_POWER (14)
#define PIN_TX    (0)
#define PIN_RX	  (1)

static struct stream_buffer_s {
	cbuffer_t *buff;
	bool ready;
} in_buff, out_buff;

void in_buff_append(char c) {
	if (c == '\r')
		in_buff.ready = true;
	cbuffer_push(in_buff.buff, &c, 1);		
}

int main(int argc, char *argv[]) {
	stdio_init_all();
	// Wait for USB serial connection
	
	in_buff.buff = cbuffer_create(255);
	in_buff.ready = false;

	out_buff.buff = cbuffer_create(255);
	out_buff.ready = false;

	uart_init(UART_INST, UART_BAUD);
	uart_set_hw_flow(UART_INST, false, false);

	gpio_init(PIN_POWER);
	gpio_set_dir(PIN_POWER, GPIO_OUT);
	//gpio_pull_up(PIN_POWER);
	gpio_put(PIN_POWER, 1);

	gpio_set_function(PIN_TX, GPIO_FUNC_UART);
	gpio_set_function(PIN_RX, GPIO_FUNC_UART);
	
	sleep_ms(5000);

	gpio_put(PIN_POWER, 0);
	sleep_ms(2500);
	gpio_put(PIN_POWER, 1);

	int read = 0;
	char command[100] = {0};
	size_t command_len = 0;
	for (;;) {
		while (!tud_cdc_connected()) { sleep_ms(100); };
		// Read from user
		if (in_buff.ready == false) {
			read = stdio_getchar_timeout_us(0);
			if (read != PICO_ERROR_TIMEOUT)
				in_buff_append(read);
		}

		if (in_buff.ready == true) {
			command_len = cbuffer_pop(in_buff.buff, 
					command, cbuffer_length(in_buff.buff));	

			uart_write_blocking(UART_INST, command, command_len);
			printf("%.*s", command_len, command);

			in_buff.ready = false;
		}

		// Read from modem
		while (uart_is_readable(UART_INST)) {
			printf("%c", uart_getc(UART_INST));
		}
	}
}
