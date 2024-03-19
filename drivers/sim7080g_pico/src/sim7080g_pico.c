#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pico/time.h"
#include "hardware/gpio.h"

#include "sim7080g_pico.h"

#define MODEM_RETRY_DELAY_MS 1000
#define MODEM_START_RETRIES 50 
#define UART_BAUD 115200
#define RX_BUFFER_SIZE 1024

static bool sim7080g_config(sim7080g_context_t *context, char *apn);

static sim7080g_context_t MODEM;
static bool MODEM_STARTED = false;

sim7080g_context_t *sim7080g_start(
	char *apn,
	uart_inst_t *uart,	
	uint pin_tx,
	uint pin_rx,
	uint pin_power
)
{
	if (MODEM_STARTED) return &MODEM;

	MODEM = (sim7080g_context_t) {0};
	static sim7080g_context_t *context = &MODEM;
	
	// Because you can actually check if uart is initialized
	// the function can accept a uart instance in any state
	if (!uart_is_enabled(uart)) 
		uart_init(uart, UART_BAUD); 

	// Disable hardware flow completely
	uart_set_hw_flow(uart, false, false);

	context->uart = uart;
	context->pin_tx = pin_tx;
	context->pin_rx = pin_rx;
	context->pin_power = pin_power;

	// gpio stuff
	gpio_init(context->pin_power);
	gpio_set_dir(context->pin_power, GPIO_OUT);
	gpio_put(context->pin_power, 0);
	gpio_set_function(context->pin_tx, GPIO_FUNC_UART);
	gpio_set_function(context->pin_rx, GPIO_FUNC_UART);

	bool success = true;
	bool power_toggled = false;
	uint8_t read_buffer[RX_BUFFER_SIZE] = {0};
	for (int tries = 0; tries < MODEM_START_RETRIES; tries++) {

		if (sim7080g_is_ready(context)) {
			success = true;
			break;
		}

		if (!power_toggled) {
			sim7080g_toggle_power(context);
			power_toggled = true;
		}

		sleep_ms(MODEM_RETRY_DELAY_MS);
		continue;
	}

	// This is how we fail
	if (!success) return NULL;
	if (!sim7080g_config(context, apn)) return NULL;

	// Wait until network is connected
	// Blocks until connected since the modem
	// is worth nothing without a network
	sim7080g_wait_for_cn(context);

	MODEM_STARTED = true;

	return context;
}

static bool sim7080g_config(sim7080g_context_t *context, char *apn) {

	if (!sim7080g_sim_ready(context)) return false;

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

	sim7080g_cb_write_blocking(context, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains_ok(rp);
}

void sim7080g_write_blocking(
		sim7080g_context_t *context,
		const uint8_t src[],
		size_t src_len
)
{
	uart_write_blocking(context->uart, src, src_len);
}

bool sim7080g_write_within_us(
		sim7080g_context_t *context, 
		const uint8_t *src, 
		size_t src_len, 
		uint64_t timeout
) 
{
	if (src_len > COMMAND_BUFFER_MAX) return false;
	
	absolute_time_t timeout_time = make_timeout_time_us(timeout);

	bool writable = false;
	while (get_absolute_time() < timeout_time) {
		if ((writable = uart_is_writable(context->uart)))
			break;

		sleep_us(WRITE_TIMEOUT_RESOLUTION_US);
	}
	if (!writable) return false;

	uart_write_blocking(context->uart, src, src_len);
	return true;
}


void sim7080g_cb_write_blocking(sim7080g_context_t *context, CommandBuffer cb[static 1]) {
	sim7080g_write_blocking(context, cb_get_buffer(cb), cb_length(cb));
}


bool sim7080g_cb_write_within_us(
		sim7080g_context_t *context, 
		CommandBuffer *cb, 
		uint64_t timeout
) 
{
	return sim7080g_write_within_us(
			context,	
			cb_get_buffer(cb),
			cb_length(cb),
			timeout
	);
}

uint32_t sim7080g_read_within_us(
		sim7080g_context_t *context, 
		uint8_t *dst, 
		size_t dst_len, 
		uint64_t timeout
) 
{
	if (!uart_is_readable_within_us(context->uart, timeout)) return 0;

	return sim7080g_read_blocking(context, dst, dst_len);
}

uint32_t sim7080g_read_blocking(sim7080g_context_t *context, uint8_t *dst, size_t dst_len) {
	if (dst == NULL) {
		dst = (uint8_t[RX_BUFFER_SIZE]) {0};
		dst_len = RX_BUFFER_SIZE;
	}

	uint8_t received = 0;
	for (uint8_t *p = dst; p - dst < dst_len; p++, received++) {
		printf("reading blocking\n");
		uart_read_blocking(context->uart, p, 1);

		if (!uart_is_readable_within_us(context->uart, READ_STOP_TIMEOUT_US)) 
			break;
	}

	return received;
}

bool sim7080g_read_blocking_ok(sim7080g_context_t *context) {

	uint8_t read_buffer[RX_BUFFER_SIZE] = {0};
	uint32_t received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains_ok(rp);
}

bool sim7080g_read_ok_within_us(sim7080g_context_t *context, uint64_t timeout) {
	if (!uart_is_readable_within_us(context->uart, timeout)) return false;

	return sim7080g_read_blocking_ok(context);
}

bool sim7080g_is_ready(sim7080g_context_t *context) {
	
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "E0", 2);

	sim7080g_cb_write_blocking(context, cb);

    return sim7080g_read_ok_within_us(context, 100 * 1000);
}


bool sim7080g_sim_ready(sim7080g_context_t *context) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	ResponseParser *rp = rp_reset(&(ResponseParser) {0});

	cb_at_prefix_set(cb);
	cb_write(cb, "+CPIN?", 6);

	sim7080g_cb_write_blocking(context, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

	rp_parse(rp, read_buffer, received);
	return rp_contains(rp, "+CPIN: READY", 12, NULL);
}


bool sim7080g_cn_available(sim7080g_context_t *context) {

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});

	cb_at_prefix_set(cb);
	cb_write(cb, "+COPS?", 6);

	sim7080g_cb_write_blocking(context, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains(rp, "+COPS: 0,", 9, NULL);
}

bool sim7080g_cn_is_active(sim7080g_context_t *context) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CNACT?", 7); 

	sim7080g_cb_write_blocking(context, cb);

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	rp_parse(rp, read_buffer, received);

	return rp_contains(rp, "+CNACT: 0,1", 11, NULL);
}

bool sim7080g_cn_activate(sim7080g_context_t *context, bool activate) {
	if (!sim7080g_cn_available(context)) return false;

	// Avoid doing anything if we are already in the right state
	if (activate && sim7080g_cn_is_active(context)) return true;
	if (!activate && !sim7080g_cn_is_active(context)) return true;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CNACT=0,", 9); 
	cb_write(cb, activate ? "1" : "0", 1);

	sim7080g_cb_write_blocking(context, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint32_t received = 0;
	uint8_t read_buffer[RX_BUFFER_SIZE];
	for (;;) {

		received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);
		rp_parse(rp, read_buffer, received);
		if (rp_contains_ok_or_err(rp)) break;

		sleep_ms(1000);
	}

	if (activate)
		return rp_contains(rp, "+APP PDP: 0,ACTIVE", 18, NULL);

	return rp_contains(rp, "+APP PDP: 0,DEACTIVE", 20, NULL);
}

bool sim7080g_ssl_enable(sim7080g_context_t *context, bool enable) {
	if (!sim7080g_cn_is_active(context)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CASSLCFG=0,\"SSL\",", 18); 
	cb_write(cb, enable ? "1" : "0", 1);

	sim7080g_cb_write_blocking(context, cb);

	return sim7080g_read_blocking_ok(context);
}

void sim7080g_wait_for_cn(sim7080g_context_t *context) {
	while(!sim7080g_cn_available(context)) sleep_ms(1000);
}

bool sim7080g_tcp_open(
		sim7080g_context_t *context, 
		uint8_t url_len,  
		uint8_t url[static url_len],
		uint16_t port
)
{
	if (!sim7080g_cn_is_active(context)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CAOPEN=0,0,\"TCP\",\"", 19); 
	cb_write(cb, url, url_len);
	cb_write(cb, "\",", 2);

	uint8_t port_str[6];
	size_t str_len = sprintf(port_str, "%u", port);
	cb_write(cb, port_str, str_len);

	sim7080g_cb_write_blocking(context, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;
	for (;;) {

		received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);
		rp_parse(rp, read_buffer, received);

		if (rp_contains_ok_or_err(rp)) break;

		sleep_ms(1000);
	}

	return rp_contains(rp, "+CAOPEN: 0,0", 12, NULL);
}

bool sim7080g_tcp_close(sim7080g_context_t *context) {
	if (!sim7080g_cn_is_active(context)) return false;

	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CACLOSE=0", 10); 
	
	sim7080g_cb_write_blocking(context, cb);

	return sim7080g_read_blocking_ok(context);
}

bool sim7080g_tcp_send(
		sim7080g_context_t *context,
		size_t data_len,
		uint8_t data[static data_len]
)
{
	//if (!sim7080g_cn_is_active(context)) return false;

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
		sim7080g_cb_write_blocking(context, cb);
		printf("writing done\n");

		printf("reading\n");
		received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);
		printf("reading done\n");

		rp_reset(rp);
		rp_parse(rp, read_buffer, received);
		
		if (!rp_contains(rp, ">", 1, NULL)) return false;

		sim7080g_write_blocking(context, p, send_len);

		p += send_len;

		received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

		rp_reset(rp);
		rp_parse(rp, read_buffer, received);

		if (!rp_contains_ok(rp)) return false;
	}

	return true;
}

size_t sim7080g_tcp_recv(
		sim7080g_context_t *context,
		size_t dst_len,
		uint8_t dst[dst_len]
)
{
	if (!sim7080g_cn_is_active(context)) return 0;

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

		sim7080g_cb_write_blocking(context, cb);
		
		received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);

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

size_t sim7080g_tcp_recv_within_us(
		sim7080g_context_t *context,
		size_t dst_len,
		uint8_t dst[dst_len],
		uint64_t timeout
)
{
	absolute_time_t timeout_time = make_timeout_time_us(timeout);

	uint32_t received = 0;
	while (get_absolute_time() < timeout_time) {
		received = sim7080g_tcp_recv(context, dst_len, dst);
		if (received) break;

		sleep_ms(50);
	}
		
	return received;
}

bool sim7080g_tcp_recv_ready_within_us(sim7080g_context_t *context, uint64_t timeout) {

	ResponseParser *rp = &(ResponseParser) {0};

	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;
	bool success = false;
	absolute_time_t timeout_time = make_timeout_time_us(timeout);
	while (get_absolute_time() < timeout_time) {

		received = sim7080g_read_within_us(context, read_buffer, RX_BUFFER_SIZE, timeout);
		rp_reset(rp);
		rp_parse(rp, read_buffer, received);

		if (rp_contains(rp, "+CADATAIND: 0", 13, NULL)) {
			success = true;
			break;
		}
	}

	return success;
}

bool sim7080g_tcp_is_open(sim7080g_context_t *context) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CASTATE?", 9);

	sim7080g_cb_write_blocking(context, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = 0;
	for (;;) {

		received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);
		rp_parse(rp, read_buffer, received);

		if (rp_contains_ok_or_err(rp)) break;
		sleep_ms(50);
	}

	return rp_contains(rp, "+CASTATE: 0,1", 13, NULL);
}

void sim7080g_read_to_null(sim7080g_context_t *context) {
	sim7080g_read_within_us(context, NULL, 0, 1000);
}

bool sim7080g_toggle_power(sim7080g_context_t *context) {
	gpio_put(context->pin_power, 1);
	sleep_ms(2500);
	gpio_put(context->pin_power, 0);
}

bool sim7080g_power_down(sim7080g_context_t *context) {
	CommandBuffer *cb = cb_reset(&(CommandBuffer) {0});
	cb_at_prefix_set(cb);
	cb_write(cb, "+CPOWD=1", 8);

	sim7080g_cb_write_blocking(context, cb);

	ResponseParser *rp = rp_reset(&(ResponseParser) {0});
	uint8_t read_buffer[RX_BUFFER_SIZE];
	uint32_t received = sim7080g_read_blocking(context, read_buffer, RX_BUFFER_SIZE);
	rp_parse(rp, read_buffer, received);

	return rp_contains(rp, "NORMAL POWER DOWN", 17, NULL);
}
