#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pico/time.h"
#include "hardware/gpio.h"

#include "modem.h"

#define MODEM_START_RETRIES 20
#define UART_BAUD 115200
#define RX_BUFFER_SIZE 1024

struct _modem {
	uart_inst_t *uart;
	uint pin_tx;
	uint pin_rx;
	uint pin_power;
};

static Modem MODEM;
static bool MODEM_STARTED = false;

Modem *modem_start(
	char *apn,
	uart_inst_t *uart,	
	uint pin_tx,
	uint pin_rx,
	uint pin_power
)
{
	// This protects us from wierd things happening
	// if we call this function twice
	if (MODEM_STARTED) return &MODEM;
	
	if (!uart_is_enabled(uart)) {
		uart_init(uart, UART_BAUD); 
		uart_set_hw_flow(uart, false, false);
	}

	MODEM.uart = uart;
	MODEM.pin_tx = pin_tx;
	MODEM.pin_rx = pin_rx;
	MODEM.pin_power = pin_power;

	gpio_init(MODEM.pin_power);
	gpio_set_dir(MODEM.pin_power, GPIO_OUT);
	gpio_put(MODEM.pin_power, 0);
	gpio_set_function(MODEM.pin_tx, GPIO_FUNC_UART);
	gpio_set_function(MODEM.pin_rx, GPIO_FUNC_UART);

	char buf[1024];
	bool reset = false;
	for (int i = 0; i < MODEM_START_RETRIES; i++) {


		bool success = false; //modem_at_send(&MODEM, "ATE0", "OK", buf, 256, 100);
		if (success) {
			MODEM_STARTED = true;

			printf("%s\n", buf);

			success = modem_config(&MODEM, apn);

			return &MODEM; // Successful return
		}
		
		sleep_ms(500);
	}

	return NULL;
}

// Sleep time between checking uart tx readiness
// which effects the resolution of the timeout time
// in exchange for less mashing on the CPU
#define WRITE_TIMEOUT_RESOLUTION_US 50
bool modem_write_within_us(
		Modem *modem, 
		const uint8_t *src, 
		size_t src_len, 
		uint32_t us
) 
{
	if (src_len > COMMAND_BUFFER_MAX) return false;
	
	absolute_time_t timeout = make_timeout_time_us(us);

	bool writable = false;
	while (get_absolute_time() < timeout) {
		if ((writable = uart_is_writable(modem->uart)))
			break;

		sleep_us(WRITE_TIMEOUT_RESOLUTION_US);
	}
	if (!writable) return false;

	uart_write_blocking(modem->uart, src, src_len);
	return true;
}

bool modem_command_write_within_us(
		Modem *modem, 
		CommandBuffer *cb, 
		uint32_t us
) 
{
	return modem_write_within_us(
			modem,	
			command_buffer_get(cb),
			command_buffer_length(cb),
			us
	);
}

#define READ_STOP_TIMEOUT_US 20
uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len, 
		uint32_t us
) 
{
	if (!uart_is_readable_within_us(modem->uart, us)) return 0;
	uint8_t received = 0;
	for (uint8_t *p = dst; p - dst < dst_len; p++, received++) {
		if (!uart_is_readable_within_us(modem->uart, READ_STOP_TIMEOUT_US)) 
			break;

		uart_read_blocking(modem->uart, p, 1);
	}

	return received;
}

uint32_t modem_read_blocking(Modem *modem, uint8_t *dst, size_t dst_len) {
	uint8_t received = 0;
	for (uint8_t *p = dst; p - dst < dst_len; p++, received++) {
		if (!uart_is_readable_within_us(modem->uart, READ_STOP_TIMEOUT_US)) 
			break;

		uart_read_blocking(modem->uart, p, 1);
	}
	
	return received;
}

static bool modem_config(Modem *modem, char *apn) {
	char buf[256];
	// +CMEE=2  Verbose errors
	// +CMGF=1  SMS message format: text
	// +CMGD=4  Clear any existing SMS messages in buffer
	// +CNMP=38 Preferred mode: LTE only
	// +CMNB=1  Preferred network: CAT-M
	bool success = false; //modem_at_send(
	//		modem,
	//		"AT+CMEE=2;+CMGF=1;+CMGD=4;+CNMP=38;+CMNB=1;",
	//		"OK",
	//		buf,
	//		256,
	//		500
	//);
	//if (!success) return false;
	//printf("%s\n", buf);

	//// Define PDP context
	//char command[256] = "AT+CGDCONT=1,\"IP\",\"";
	//strcat(command, apn);
	//strcat(command, "\"");

	//success = modem_at_send(
	//		modem,
	//		command,
	//		"OK",
	//		buf,
	//		256,
	//		500
	//);
	//if (!success) return false;
	//printf("%s\n", buf);

	return success;
}


bool modem_toggle_power(Modem *modem) {
	gpio_put(modem->pin_power, 1);
	sleep_ms(2500);
	gpio_put(modem->pin_power, 0);
}

static void _gpio_init(Modem *modem) {
}
