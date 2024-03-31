#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "sim7080g_pico.h"

#include "gw_core_error.h"
#include "gateway_codes.h"
#include "gateway_queue.h"

#define UART_BAUD 115200

#define COMMAND_DISPATCHED (0)
#define COMMAND_INCOMPLETE (1)
#define COMMAND_BUFFER_FULL (-1)
#define COMMAND_UNRECOGNIZED (-2)

#define SERVER_IP "34.69.199.172"

typedef enum _modem_state {
	MODEM_UNINITIALIZED,
	MODEM_STOPPED,
	MODEM_STARTED,
	MODEM_CN_ACTIVE,
	MODEM_SERVER_CONNECTED
} MODEM_STATE_T;

static sim7080g_context_t *_gateway = NULL;
static MODEM_STATE_T _modem_state = MODEM_UNINITIALIZED;
static bool _modem_power_toggled = false;

// Command flags
static bool _start_command_issued = false;

// Data output buffer
#define MODEM_BUFFER_OUT_SIZE (1024 * 10) // 10 KB
static uint8_t _modem_buffer_out[MODEM_BUFFER_OUT_SIZE] = {0};
static uint8_t *_buffer_out_end = &_modem_buffer_out[MODEM_BUFFER_OUT_SIZE];
static uint8_t *_buffer_out_p = _modem_buffer_out;

// Command buffer
// Should only need to hold several commands at a time
#define MODEM_BUFFER_COMMAND_SIZE (sizeof (uint32_t) * 100)
static uint8_t _modem_buffer_command[MODEM_BUFFER_COMMAND_SIZE];
static uint8_t *_buffer_command_end = &_modem_buffer_command[MODEM_BUFFER_COMMAND_SIZE];
static uint8_t *_buffer_command_p = _modem_buffer_command;

// Internal func prototypes
static bool _modem_cn_available(void);
static int _message_queue_process(void);
static void _dispatch_message_buffer(uint32_t buffer);
static int _build_command(uint32_t buffer);
static bool _command_buffer_push(void *command, uint size);
static void _command_buffer_execute(void);

uint16_t htons(uint16_t num) {
	uint16_t output = 0;
	for (int i = 0; i < 16; i += 8)
		output |= ((num >> i) & 0xFF) << (16 - 8 - i);	
	return output;
};

uint32_t htonl(uint32_t num) {
	uint32_t output = 0;
	for (int i = 0; i < 32; i += 8)
		output |= ((num >> i) & 0xFF) << (16 - 8 - i);	
	return output;
};

static void gw_core_panic(GW_CORE_ERROR_T error) {
	gw_core_error_set(error);
	for (;;) {
		printf("%s\n", gw_core_error_str);
		sleep_ms(3000);
	}
}

void gw_core_entry(void) {

	bool success = false;
	char *error = "ok";

	_gateway = sim7080g_create();
	if (_gateway == NULL) {
		error = "sim7080g_create returned NULL";
		goto LOOP_BEGIN;
	}

	sim7080g_init(
			_gateway,
			GATEWAY_APN,
			GATEWAY_UART,
			GATEWAY_PIN_TX,
			GATEWAY_PIN_RX,
			GATEWAY_PIN_PWR
	);

	_modem_state = MODEM_STOPPED;

	success = true;
LOOP_BEGIN:
	if (success == false) {
		// Errors at this point are unrecoverable	
		// so we send our error to the main process
		// and then go into a dead loop.

	}

	// Main loop
	for (;;) {
		_message_queue_process();
		_command_buffer_execute();

		// If the modem is off and we have been issued a 
		if (_start_command_issued && _modem_state == MODEM_STOPPED) {
			if (sim7080g_is_ready(_gateway) && sim7080g_config(_gateway)) {
				_modem_state = MODEM_STARTED;
				printf("modem started\n");
			}
			else if (_modem_power_toggled == false) {
				printf("togging power\n");
				sim7080g_toggle_power(_gateway);
				_modem_power_toggled = true;
			}
		} 
		else if (_start_command_issued) _start_command_issued = false;
		else if (_modem_state == MODEM_STARTED)
			if (sim7080g_is_ready(_gateway) == false)
				_modem_state = MODEM_STOPPED;

		// If we are in a powered on state and a CN is available
		// activate it
		if (_modem_state == MODEM_STARTED && _modem_cn_available()) {
			printf("trying to activate\n");
			if (sim7080g_cn_activate(_gateway, true)) {
				_modem_state = MODEM_CN_ACTIVE;
				printf("cn activated\n");
			}
		}
		// It the CN is active, connect to server
		else if (_modem_state == MODEM_CN_ACTIVE)
			if (sim7080g_cn_is_active(_gateway) == false)
				_modem_state = MODEM_STARTED;


		if (_modem_state == MODEM_CN_ACTIVE && sim7080g_ssl_enable(_gateway,false)) {
			sleep_ms(1000);
			if (sim7080g_tcp_open(_gateway, strlen(SERVER_IP), SERVER_IP, 8086))
				_modem_state = MODEM_SERVER_CONNECTED;
		}

		else if (_modem_state == MODEM_SERVER_CONNECTED) {
			char *message = "Hello!";
			uint16_t packet[10] = {[0] = htons(0), [1] = htons(strlen(message))};
			memcpy(&packet[2], message, strlen(message));
			sim7080g_tcp_send(_gateway, strlen(message) + (sizeof (uint16_t) * 2), (uint8_t *)packet);
			printf("sent\n");
		}
		printf("end\n");


//	if (modem_tcp_open(modem, strlen(SERVER_URL), SERVER_URL, SERVER_PORT)) {
//		printf("TCP connection opened\n");
//	}
//
//	uint8_t *msg = "PING";
//	if (modem_tcp_send(modem, strlen(msg), msg)) printf("Data sent!\n");

		printf("cn available: %s\n",
				_modem_cn_available() ? "true" : "false");
		printf("state: %u\n", _modem_state);

LOOP_CONTINUE:
		sleep_ms(1000);
	}
}

uint gw_core_buffer_out_remaining(void) {
	return _buffer_out_end - _buffer_out_p;
}

static bool _modem_cn_available(void) {
	if (_modem_state ==  MODEM_UNINITIALIZED || _modem_state == MODEM_STOPPED)
		return false;

	return sim7080g_cn_available(_gateway);
}

#define GW_Q_BUF_SIZE 400
#define GW_Q_BUF_MALLOC_FAILED -6
static int _message_queue_process(void) {
	uint32_t buffer = 0;
	uint16_t received = 0;
	uint16_t new_received;
	while (new_received = gw_queue_gw_recv((uint8_t *)&buffer, (sizeof buffer))) {
		if (new_received < 0) gw_core_panic(GW_CORE_FAILURE);

		received += new_received;
		_dispatch_message_buffer(buffer);
	}
	
	return received;
}

static void _dispatch_message_buffer(uint32_t buffer) {
	static bool new_message = true;
	static uint32_t message_type;

	if (new_message) {
		message_type = buffer;
		new_message = false;
	} else {
		uint rval = 0;
		switch(message_type) {
		case GATEWAY_COMMAND:
			// Unrecoverable error if rval < 0
			rval == _build_command(buffer);
			if (rval < 0) gw_core_panic(GW_CORE_FAILURE);
			if (rval == COMMAND_DISPATCHED) new_message = true;
			break;
		case PACKED_DATA:
			break;
		}
	}
}

static int _build_command(uint32_t buffer) {
	static bool new_command = true;
	static uint32_t command_type;

	if (new_command) {
		command_type = buffer;
		new_command = false;
	}

	uint rval = COMMAND_UNRECOGNIZED;
	bool success = false;
	switch (command_type) {
	case GATEWAY_START:
    case GATEWAY_STOP:
		if (_command_buffer_push(&buffer, (sizeof buffer)))
			rval = COMMAND_DISPATCHED;
		else rval = COMMAND_BUFFER_FULL;
		break;
	}

	if (rval == COMMAND_DISPATCHED) new_command = true;

	return rval;
}

static bool _command_buffer_push(void *command, uint size) {
	if (size > (_buffer_command_end - _buffer_command_p)) return false;	

	uint8_t *cp = (uint8_t *)command;
	for (int i = 0; i < size; i++) {
		*_buffer_command_p = cp[i];
		_buffer_command_p++;
	}

	return true;
}

static void _command_buffer_execute(void) {
	uint8_t *bp = _modem_buffer_command;
	bool new_command = true;
	uint32_t command_type;
	while (bp != _buffer_command_p) {
		if (new_command) {
			command_type = *(uint32_t *)bp;
			bp += sizeof (uint32_t);
			new_command = false;
		}

		bool success = false;
		switch (command_type) {
		case GATEWAY_START:
			_start_command_issued = true;
			new_command = true;
			break;
		case GATEWAY_STOP:
			if (sim7080g_power_down(_gateway)) _modem_state = MODEM_STOPPED;
			new_command = true;
			break;	
		}
	}

	// Reset global buffer pointer
	_buffer_command_p = _modem_buffer_command;
}

//void modem_test(void) {
//	printf("Starting modem... ");
//
//	Modem *modem = modem_start(
//			MODEM_APN,
//			UART_PORT,
//			UART_PIN_TX,
//			UART_PIN_RX,
//			MODEM_PIN_PWR
//	);
//
//	if (modem) printf("success!\n");
//	else {
//		printf("fail\n");
//		return;
//	}
//	
//	if (modem_cn_activate(modem, true)) printf("Network activated\n");
//
//	if (modem_ssl_enable(modem, false)) printf("SSL disabled\n");
//
//	if (modem_tcp_open(modem, strlen(SERVER_URL), SERVER_URL, SERVER_PORT)) {
//		printf("TCP connection opened\n");
//	}
//
//	uint8_t *msg = "PING";
//	if (modem_tcp_send(modem, strlen(msg), msg)) printf("Data sent!\n");
//
//	if (modem_tcp_recv_ready_within_us(modem, 1000 * 1000 * 10)) {
//		uint8_t dst[100] = {0};
//		size_t received = modem_tcp_recv(modem, 100, dst);
//		printf("received: %u\n", received);
//		printf("%.*s\n", received, dst);
//	}
//
//	if (modem_tcp_close(modem)) printf("TCP connection closed\n");
//
//	if (modem_cn_activate(modem, false)) printf("Network deactivated\n");
//
//	if (modem_power_down(modem)) printf("Modem powered down\n");
//}
