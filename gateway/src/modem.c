#include <stdlib.h>
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

static bool modem_config(Modem *modem, char *apn);

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

	bool success = true;
	bool power_toggled = false;
	uint8_t read_buffer[RX_BUFFER_SIZE] = {0};
	for (int tries = 0; tries < MODEM_START_RETRIES; tries++) {

		if (modem_is_ready(modem)) {
			success = true;
			break;
		}

		if (!power_toggled) {
			modem_toggle_power(modem);
			power_toggled = true;
		}

		sleep_ms(MODEM_RETRY_DELAY_MS);
		continue;
	}

	// This is how we fail
	if (!success) return NULL;
	if (!modem_config(modem, apn)) return NULL;

	// Wait until network is connected
	// Blocks until connected since the modem
	// is worth nothing without a network
	modem_wait_for_cn(modem);

	MODEM_STARTED = true;

	return modem;
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
	// +CNCFG   Restr_puest proper code from carrier network
	uint8_t *command = 
		"+CMEE=2;+CMGF=1;+CMGD=,4;+CNMP=38;+CMNB=1;+CGDCONT=1,\"IP\",\"";
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, command, strlen(command));
	cb_write(cb, apn, strlen(apn));
	cb_write(cb, "\"", 1);

	modem_cb_write_blocking(modem, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains_ok(rp);
}

void modem_write_blocking(
		Modem *modem,
		const uint8_t src[],
		size_t src_len
)
{
	uart_write_blocking(modem->uart, src, src_len);
}

bool modem_write_within_us(
		Modem *modem, 
		const uint8_t *src, 
		size_t src_len, 
		uint64_t timeout
) 
{
	if (src_len > COMMAND_BUFFER_MAX) return false;
	
	absolute_time_t timeout_time = make_timeout_time_us(timeout);

	bool writable = false;
	while (get_absolute_time() < timeout_time) {
		if ((writable = uart_is_writable(modem->uart)))
			break;

		sleep_us(WRITE_TIMEOUT_RESOLUTION_US);
	}
	if (!writable) return false;

	uart_write_blocking(modem->uart, src, src_len);
	return true;
}


void modem_cb_write_blocking(Modem *modem, CommandBuffer cb[static 1]) {
	modem_write_blocking(modem, cb_get_buffer(cb), cb_length(cb));
}


bool modem_cb_write_within_us(
		Modem *modem, 
		CommandBuffer *cb, 
		uint64_t timeout
) 
{
	return modem_write_within_us(
			modem,	
			cb_get_buffer(cb),
			cb_length(cb),
			timeout
	);
}

uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len, 
		uint64_t timeout
) 
{
	if (!uart_is_readable_within_us(modem->uart, timeout)) return 0;

	return modem_read_blocking(modem, dst, dst_len);
}

uint32_t modem_read_blocking(Modem *modem, uint8_t *dst, size_t dst_len) {
	if (dst == NULL) {
		dst = (uint8_t[RX_BUFFER_SIZE]) {0};
		dst_len = RX_BUFFER_SIZE;
	}

	uint8_t received = 0;
	for (uint8_t *p = dst; p - dst < dst_len; p++, received++) {
		printf("reading blocking\n");
		uart_read_blocking(modem->uart, p, 1);

		if (!uart_is_readable_within_us(modem->uart, READ_STOP_TIMEOUT_US)) 
			break;
	}

	return received;
}

bool modem_read_blocking_ok(Modem *modem) {

	uint8_t read_buffer[RX_BUFFER_SIZE] = {0};
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains_ok(rp);
}

bool modem_read_ok_within_us(Modem *modem, uint64_t timeout) {
	if (!uart_is_readable_within_us(modem->uart, timeout)) return false;

	return modem_read_blocking_ok(modem);
}

bool modem_is_ready(Modem *modem) {
	
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "E0", 2);

	modem_cb_write_blocking(modem, cb);

    return modem_read_ok_within_us(modem, 100 * 1000);
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


bool modem_cn_available(Modem *modem) {

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});

	cb_at_prefix_set(cb);
	cb_write(cb, "+COPS?", 6);

	modem_cb_write_blocking(modem, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains(rp, "+COPS: 0,", 9, NULL);
}

bool modem_cn_is_active(Modem *modem) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CNACT?", 7); 

	modem_cb_write_blocking(modem, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains(rp, "+CNACT: 0,1", 11, NULL);
}

bool modem_cn_activate(Modem *modem, bool activate) {
	if (!modem_cn_available(modem)) return false;

	// Avoid doing anything if we are already in the right state
	if (activate && modem_cn_is_active(modem)) return true;
	if (!activate && !modem_cn_is_active(modem)) return true;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CNACT=0,", 9); 
	cb_write(cb, activate ? "1" : "0", 1);

	modem_cb_write_blocking(modem, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint32_t received = 0;
	uint8_t read_buffer[RX_BUFFER_SIZE];
	for (;;) {

		received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);
		rp_parse(rp, read_buffer, received);
		if (rp_contains_ok_or_err(rp)) break;

		sleep_ms(1000);
	}

	if (activate)
		return rp_contains(rp, "+APP PDP: 0,ACTIVE", 18, NULL);

	return rp_contains(rp, "+APP PDP: 0,DEACTIVE", 20, NULL);
}

bool modem_ssl_enable(Modem *modem, bool enable) {
	if (!modem_cn_is_active(modem)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CASSLCFG=0,\"SSL\",", 18); 
	cb_write(cb, enable ? "1" : "0", 1);

	modem_cb_write_blocking(modem, cb);

	return modem_read_blocking_ok(modem);
}

void modem_wait_for_cn(Modem *modem) {
	while(!modem_cn_available(modem)) sleep_ms(1000);
}

bool modem_tcp_open(
		Modem *modem, 
		uint8_t url_len,  
		uint8_t url[static url_len],
		uint16_t port
)
{
	if (!modem_cn_is_active(modem)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CAOPEN=0,0,\"TCP\",\"", 19); 
	cb_write(cb, url, url_len);
	cb_write(cb, "\",", 2);

	uint8_t port_str[6];
	size_t str_len = sprintf(port_str, "%u", port);
	cb_write(cb, port_str, str_len);

	modem_cb_write_blocking(modem, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;
	for (;;) {

		received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);
		rp_parse(rp, read_buffer, received);

		if (rp_contains_ok_or_err(rp)) break;

		sleep_ms(1000);
	}

	return rp_contains(rp, "+CAOPEN: 0,0", 12, NULL);
}

bool modem_tcp_close(Modem *modem) {
	if (!modem_cn_is_active(modem)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CACLOSE=0", 10); 
	
	modem_cb_write_blocking(modem, cb);

	return modem_read_blocking_ok(modem);
}

bool modem_tcp_send(
		Modem *modem,
		size_t data_len,
		uint8_t data[static data_len]
)
{
	//if (!modem_cn_is_active(modem)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	ResponseParser *rp = rp_reset(&(ResponseParser) {0});

	size_t send_len;
	uint8_t command[100];
	uint8_t command_len;
	uint8_t *p = data;
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received;

	while(data_len) {
		if (data_len > MODEM_TCP_SEND_MAX)
			send_len = MODEM_TCP_SEND_MAX;
		else
			send_len = data_len;

		data_len -= send_len;

		command_len = sprintf(command, "+CASEND=0,%u", send_len);

		cb_reset(cb);
		cb_at_prefix_set(cb);
		cb_write(cb, command, command_len);

		printf("writing\n");
		modem_cb_write_blocking(modem, cb);
		printf("writing done\n");

		printf("reading\n");
		received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);
		printf("reading done\n");

		rp_reset(rp);
		rp_parse(rp, read_buffer, received);
		
		if (!rp_contains(rp, ">", 1, NULL)) return false;

		modem_write_blocking(modem, p, send_len);

		p += send_len;

		received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

		rp_reset(rp);
		rp_parse(rp, read_buffer, received);

		if (!rp_contains_ok(rp)) return false;
	}

	return true;
}

size_t modem_tcp_recv(
		Modem *modem,
		size_t dst_len,
		uint8_t dst[dst_len]
)
{
	if (!modem_cn_is_active(modem)) return 0;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	ResponseParser *rp = rp_reset(&(ResponseParser) {0});

	size_t recv_len;
	uint8_t command[100] = {0};
	size_t command_len = 0;
	uint8_t *dst_p = dst;
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received;
	size_t total_received = 0;
	uint8_t len_str[6];
	uint8_t *str_p = NULL;
	uint16_t data_len = 0;
	uint8_t *r = NULL;
	while (dst_len - total_received) {
		if (dst_len - total_received > MODEM_TCP_SEND_MAX)
			recv_len = MODEM_TCP_SEND_MAX;
		else
			recv_len = dst_len - total_received;

		command_len = sprintf(command, "+CARECV=0,%u", recv_len);

		cb_reset(cb);
		cb_at_prefix_set(cb);
		cb_write(cb, command, command_len);

		modem_cb_write_blocking(modem, cb);
		
		received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);

		rp_reset(rp);
		rp_parse(rp, read_buffer, received);

		uint8_t index;
		if (!rp_contains(rp, "+CARECV: ", 9, &index)) return 0; 
	
		uint8_t *response; 
		uint32_t response_len;
		if (!rp_get(rp, index, &response, &response_len)) return 0;

		// No data
		if (response[9] == '0') break;
		
		str_p = len_str;	
		for (r = &response[9]; *r != ','; r++) {
			*str_p++ = *r;	
		}
		*str_p = '\0';
		// set r to first byte of data
		r++;
		data_len = atoi(len_str);

		for (int i = 0; i < data_len; i++) {
			if (total_received == dst_len) break;
			*dst_p++ = *r++;
			total_received++;
		}
	}

	return total_received;
}

size_t modem_tcp_recv_within_us(
		Modem *modem,
		size_t dst_len,
		uint8_t dst[dst_len],
		uint64_t timeout
)
{
	absolute_time_t timeout_time = make_timeout_time_us(timeout);

	uint32_t received = 0;
	while (get_absolute_time() < timeout_time) {
		received = modem_tcp_recv(modem, dst_len, dst);
		if (received) break;

		sleep_ms(50);
	}
		
	return received;
}

bool modem_tcp_recv_ready_within_us(Modem *modem, uint64_t timeout) {

	ResponseParser *rp = &(ResponseParser) {0};

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;
	bool success = false;
	absolute_time_t timeout_time = make_timeout_time_us(timeout);
	while (get_absolute_time() < timeout_time) {

		received = modem_read_within_us(modem, read_buffer, RX_BUFFER_SIZE, timeout);
		rp_reset(rp);
		rp_parse(rp, read_buffer, received);

		if (rp_contains(rp, "+CADATAIND: 0", 13, NULL)) {
			success = true;
			break;
		}
	}

	return success;
}

bool modem_tcp_is_open(Modem *modem) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CASTATE?", 9);

	modem_cb_write_blocking(modem, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;
	for (;;) {

		received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);
		rp_parse(rp, read_buffer, received);

		if (rp_contains_ok_or_err(rp)) break;
		sleep_ms(50);
	}

	return rp_contains(rp, "+CASTATE: 0,1", 13, NULL);
}

void modem_read_to_null(Modem *modem) {
	modem_read_within_us(modem, NULL, 0, 1000);
}

bool modem_toggle_power(Modem *modem) {
	gpio_put(modem->pin_power, 1);
	sleep_ms(2500);
	gpio_put(modem->pin_power, 0);
}

bool modem_power_down(Modem *modem) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CPOWD=1", 8);

	modem_cb_write_blocking(modem, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = modem_read_blocking(modem, read_buffer, RX_BUFFER_SIZE);
	rp_parse(rp, read_buffer, received);

	return rp_contains(rp, "NORMAL POWER DOWN", 17, NULL);
}
