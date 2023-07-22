#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pico/time.h"
#include "hardware/gpio.h"

#include "modem.h"

#define MODEM_RETRY_DELAY_MS 1000
#define MODEM_START_RETRIES 50 
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


	bool reset = false;

	CommandBuffer *cb = cb_create();
	cb_prefix_set(cb);
	cb_write(cb, "E0", 2);

	ResponseParser *rp = rp_create();

	size_t read_buffer_len = 1024;
	uint8_t read_buffer[read_buffer_len];

	// Clear RX
	uint64_t timeout_us = 100;
	while(uart_is_readable(MODEM.uart))
		modem_read_within_us(&MODEM, read_buffer, read_buffer_len, timeout_us);

	Modem *modem = NULL;
	bool power_toggled = false;
	for (int i = 0; i < MODEM_START_RETRIES; i++) {

		timeout_us = 1000 * 10; // 10 ms
		bool success = modem_command_write_within_us(&MODEM, cb, timeout_us);
		if (!success) {
			sleep_ms(MODEM_RETRY_DELAY_MS); 
			continue;
		}

		timeout_us = 1000 * 100; // 100 ms
		uint32_t received = modem_read_within_us(
				&MODEM, 
				read_buffer, 
				read_buffer_len, 
				1000 * 100
		);

		if (!received) {
			if (!power_toggled) {
				modem_toggle_power(&MODEM);
				power_toggled = true;
			}
			sleep_ms(MODEM_RETRY_DELAY_MS);
			continue;
		}

		rp_clear(rp);
		rp_parse(rp, read_buffer, received);
		if (!rp_contains_ok(rp)) break;

		if (!modem_config(&MODEM, apn)) break;

		MODEM_STARTED = true;
		modem = &MODEM; // Successful return
		break;
	}

	cb_destroy(cb);
	rp_destroy(rp);

	return modem;
}

// Sleep time between checking uart tx readiness
// which effects the resolution of the timeout time
// in exchange for less mashing on the CPU
#define WRITE_TIMEOUT_RESOLUTION_US 50
bool modem_write_within_us(
		Modem *modem, 
		const uint8_t *src, 
		size_t src_len, 
		uint64_t us
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
		uint64_t us
) 
{
	return modem_write_within_us(
			modem,	
			cb_get(cb),
			cb_length(cb),
			us
	);
}

#define READ_STOP_TIMEOUT_US 1000
uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len, 
		uint64_t us
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
	bool success = false;
	// +CMEE=2  Verbose errors
	// +CMGF=1  SMS message format: text
	// +CMGD=4  Clear any existing SMS messages in buffer
	// +CNMP=38 Preferred mode: LTE only
	// +CMNB=1  Preferred network: CAT-M
	CommandBuffer *cb = cb_create();
	ResponseParser *rp = rp_create();
	char *command = "+CMEE=2;+CMGF=1;+CMGD=4;+CNMP=38;+CMNB=1;+CGDCONT=1,\"IP\",\"";
	cb_prefix_set(cb);
	cb_write(cb, command, strlen(command));
	cb_write(cb, apn, strlen(apn));
	cb_write(cb, "\"", 1);

	if (!modem_command_write_within_us(modem, cb, 100 * 1000)) {
		goto CLEANUP;
	}

	uint8_t read_buffer[1024];
	uint32_t received = modem_read_within_us(
			modem, 
			read_buffer, 
			1024,
			1000 * 100
	);

	if (!received) goto CLEANUP;

	rp_clear(rp);
	rp_parse(rp, read_buffer, received);

	if (rp_contains_ok(rp)) success = true;

	
CLEANUP:
	cb_destroy(cb);
	rp_destroy(rp);
	return success;
}


bool modem_toggle_power(Modem *modem) {
	gpio_put(modem->pin_power, 1);
	sleep_ms(2500);
	gpio_put(modem->pin_power, 0);
}
