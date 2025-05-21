#include <stdio.h>
#include "string.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

#include "cbuffer.h"
#include "sim7080g_rp2x.h"

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
	
	uint8_t storage_in[255] = {0};
	cbuffer_t buffer_in;
	cbuffer_init(&buffer_in, storage_in, 255);
	in_buff.buff = &buffer_in;
	in_buff.ready = false;

	uint8_t storage_out[255] = {0};
	cbuffer_t buffer_out;
	cbuffer_init(&buffer_out, storage_out, 255);
	out_buff.buff = &buffer_out;
	out_buff.ready = false;

	uart_init(UART_INST, UART_BAUD);
	uart_set_hw_flow(UART_INST, false, false);
	gpio_set_function(PIN_TX, GPIO_FUNC_UART);
	gpio_set_function(PIN_RX, GPIO_FUNC_UART);

	uint8_t mbuf[255] = {0};
	sim7080g_context_t modem = {0};
	struct sim7080g_config config = {
		.uart_index = UART_NUM(UART_INST),
		.pin_pwr = PIN_POWER,
	};
	sim7080g_init(&modem, config);

	int read = 0;
	char command[103] = {0};
	size_t command_len = 0;
	for (;;) {
		// Read from user
		if (in_buff.ready == false) {
			read = stdio_getchar_timeout_us(0);
			if (read != PICO_ERROR_TIMEOUT)
				in_buff_append(read);
		}

		if (in_buff.ready == true) {
			command_len = cbuffer_pop(in_buff.buff, 
					command, cbuffer_length(in_buff.buff));	
			if (!strncmp(command, "toggle-power", 12)) {
				printf("toggling power\n");
				fflush(stdout);
				sim7080g_pwr_toggle(&modem);
			}  
			else if (!strncmp(command, "cn activate", 11)) {
				sim7080g_cn_activate(&modem, 0, true);	
			}
			else if (!strncmp(command, "cn deactivate", 13)) {
				sim7080g_cn_activate(&modem, 0, false);	
			}
			else if (!strncmp(command, "cn state", 8)) {
				sim7080g_cn_state(&modem);
			}
			else {
				sim7080g_uart_write_all(&modem, command, command_len);
			}
			in_buff.ready = false;
		}

		// Read from modem
		uint8_t response[500] = {0};
		int read = 0;
		while (read = sim7080g_uart_read(&modem, response, 500))
			printf("%.*s", read, response);

		fflush(stdout);
	}
}
