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

	MODEM = (Modem) {0};
	static Modem *modem = &MODEM;
	
	// Because you can actually check if uart is initialized
	// the function can accept a uart instance in any state
	if (!uart_is_enabled(uart)) 
		uart_init(uart, UART_BAUD); 

	// Disable hardware flow completely
	uart_set_hw_flow(uart, false, false);

	modem->uart = uart;
	modem->pin_tx = pin_tx;
	modem->pin_rx = pin_rx;
	modem->pin_power = pin_power;

	// gpio stuff
	gpio_init(modem->pin_power);
	gpio_set_dir(modem->pin_power, GPIO_OUT);
	gpio_put(modem->pin_power, 0);
	gpio_set_function(modem->pin_tx, GPIO_FUNC_UART);
	gpio_set_function(modem->pin_rx, GPIO_FUNC_UART);

	// Make our two helpful objects
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});

	// The command we use to test if the modem is responding
	// which also helpfully disables command echoing
	cb_at_prefix_set(cb);
	cb_write(cb, "E0", 2);

	bool power_toggled = false;
	uint8_t read_buffer[RX_BUFFER_SIZE] = {0};
	for (int tries = 0; tries < MODEM_START_RETRIES; tries++) {

		modem_cb_write_blocking(modem, cb);

		if (!modem_read_blocking_ok(modem, read_buffer, RX_BUFFER_SIZE)) {
			if (!power_toggled) {
				modem_toggle_power(modem);
				power_toggled = true;
			}
			sleep_ms(MODEM_RETRY_DELAY_MS);
			continue;
		}
		
		if (!modem_config(modem, apn)) break;
		if (!modem_connect(modem)) break;

		MODEM_STARTED = true;
		modem = modem; // Successful return
		break;
	}

	return modem;
}

void modem_write_blocking(
		Modem modem[static 1],
		const uint8_t src[],
		size_t src_len
)
{
	uart_write_blocking(modem->uart, src, src_len);
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


void modem_cb_write_blocking(Modem modem[static 1], CommandBuffer cb[static 1]) {
	modem_write_blocking(modem, cb_get_buffer(cb), cb_length(cb));
}


bool modem_cb_write_within_us(
		Modem *modem, 
		CommandBuffer *cb, 
		uint64_t us
) 
{
	return modem_write_within_us(
			modem,	
			cb_get_buffer(cb),
			cb_length(cb),
			us
	);
}

#define READ_STOP_TIMEOUT_US (1000 * 10)
uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len, 
		uint64_t us
) 
{
	if (!uart_is_readable_within_us(modem->uart, us)) return 0;

	return modem_read_blocking(modem, dst, dst_len);
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

bool modem_read_blocking_ok(Modem modem[static 1], uint8_t dst[], size_t dst_len) {

	uint32_t received = modem_read_blocking(modem, dst, dst_len);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, dst, received);
	return rp_contains_ok(rp);
}

bool modem_sim_ready(Modem *modem) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	ResponseParser *rp = rp_reset(&(ResponseParser) {0});

	cb_at_prefix_set(cb);
	cb_write(cb, "+CPIN?", 6);

	modem_cb_write_blocking(modem, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);
	
	rp_parse(rp, read_buffer, received);
	return rp_contains(rp, "+CPIN: READY", 12, NULL);
}

bool modem_is_ready(Modem modem[static 1]) {
	
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "E0", 2);

    return modem_read_blocking_ok(modem, read_buffer, RX_BUFFER_SIZE);
}

static bool modem_config(Modem *modem, char *apn) {
	if (!modem_sim_ready(modem)) return false;

	// CONFIGURING THE FOLLOWING:
	// +CMEE=2  Verbose errors
	// +CMGF=1  SMS message format: text
	// +CMGD=4  Clear any existing SMS messages in buffer
	// +CNMP=38 Preferred mode: LTE only
	// +CMNB=1  Preferred network: CAT-M
	// +CGDCONT Set APN
	// +CNCFG   Request proper code from carrier network
	uint8_t *command = 
		"+CMEE=2;+CMGF=1;+CMGD=,4;+CNMP=38;+CMNB=1;+CGDCONT=1,\"IP\",\"iot.1nce.net\"";
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, command, strlen(command));
	//cb_write(cb, apn, strlen(apn));
	//cb_write(cb, "\"", 1);
	//command = "+CNCFG=0,1,\"\"";
	//cb_write(cb, command, strlen(command));
	modem_cb_write_blocking(modem, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	if (!rp_contains_ok(rp)) return false;

	return true;
}

bool modem_connect(Modem *modem) {
	bool success = false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	ResponseParser *rp = rp_reset(&(ResponseParser) {0});

	uint8_t *command = "+COPS?";
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;

	cb_reset(cb);
	cb_at_prefix_set(cb);
	cb_write(cb, command, strlen(command));
	while(!modem_cb_write_within_us(modem, cb, 1000 * 1000))
		sleep_us(1000 * 1000 * 5);
	// Wait until we receive something
	while(!(received = modem_read_within_us(modem, read_buffer, RX_BUFFER_SIZE, 1000 * 100)))
		sleep_us(1000 * 1000 * 5);

	rp_reset(rp); rp_parse(rp, read_buffer, received);
	printf("%.*s\n", received, read_buffer);

	uint8_t *command2 = "+COPS?";

	uint8_t index;
	while(!rp_contains(rp, ",", 1, &index)) {

		cb_reset(cb);
		cb_at_prefix_set(cb);
		cb_write(cb, command2, strlen(command));
		while(!modem_cb_write_within_us(modem, cb, 1000 * 1000))
			sleep_us(1000 * 1000 * 5);
		// Wait until we receive something
		while(!(received = modem_read_within_us(modem, read_buffer, RX_BUFFER_SIZE, 1000 * 100)))
			sleep_us(1000 * 1000 * 5);

		rp_reset(rp); rp_parse(rp, read_buffer, received);
		printf("%.*s\n", received, read_buffer);
		sleep_us(1000 * 1000 * 5);
	}

	return success;
}

bool modem_toggle_power(Modem *modem) {
	gpio_put(modem->pin_power, 1);
	sleep_ms(2500);
	gpio_put(modem->pin_power, 0);
}
