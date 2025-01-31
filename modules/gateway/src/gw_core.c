//#include <stdio.h>

#include "pico/stdlib.h"


#include "gateway_codes.h"
#include "gateway_queue.h"

#define UART_BAUD 115200

#define COMMAND_DISPATCHED (0)
#define COMMAND_INCOMPLETE (1)
#define COMMAND_BUFFER_FULL (2)
#define COMMAND_UNRECOGNIZED (3)

#define PACKED_INCOMPLETE (0)
#define PACKED_COMPLETE (1)

#define SERVER_IP "73.149.88.183"


// Yup, a global for exposing state. Don't yell at me.
int MODEM_CORE_STATE = MODEM_UNINITIALIZED;

static bool _modem_power_down = false;
static bool _modem_power_toggled = false;
static bool _modem_ssl_configured = false;
static bool _modem_send_incomplete = false;

// Command flags
static bool _start_command_issued = false;

// Data output buffer
#define MODEM_BUFFER_OUT_SIZE (1024 * 10) // 10 KB
static cbuffer_t *_modem_buffer_out = NULL;

// Command buffer
// Should only need to hold several commands at a time
#define MODEM_BUFFER_COMMAND_SIZE (sizeof (uint32_t) * 100)
static cbuffer_t *_modem_buffer_command = NULL; // Internal func prototypes
static bool _modem_cn_available(void);
static int _message_queue_process(void);
static void _dispatch_message_buffer(uint32_t buffer);
static int _build_command(uint32_t buffer);
static int _build_packed(uint32_t buffer);
static bool _command_buffer_push(void *command, uint size);
static bool _modem_buffer_push(void *buffer, uint size);
static void _command_buffer_execute(void);
static bool _modem_buffer_send(void);

uint16_t htons(uint16_t num) {
	uint16_t output = 0;
	for (int i = 0; i < 16; i += 8)
		output |= ((num >> i) & 0xFF) << (16 - 8 - i);	
	return output;
};

uint32_t htonl(uint32_t num) {
	uint32_t output = 0;
	for (int i = 0; i < 32; i += 8)
		output |= ((num >> i) & 0xFF) << (32 - 8 - i);	
	return output;
};

static void gw_core_panic(GW_CORE_ERROR_T error) {
	gw_core_error_set(error);
	for (;;) {
		//printf("%s\n", gw_core_error_str);
		sleep_ms(3000);
	}
}

int gateway_pump(void) {


	// printf("Heading into main loop\n");

	// Main loop
	for (;;) {
		//_message_queue_process();
		//_command_buffer_execute();

		switch (MODEM_CORE_STATE) {
		case MODEM_POWERED_DOWN:
			if (cbuffer_empty(_modem_buffer_out))
				break;

			sim7080g_toggle_power(_gateway);
			MODEM_CORE_STATE = MODEM_STOPPED;
			break;

		case MODEM_STOPPED:
			
			// If we are stopped and have nothing to do
			if (cbuffer_empty(_modem_buffer_out)) {
				if (sim7080g_power_down(_gateway))
					MODEM_CORE_STATE = MODEM_POWERED_DOWN;

				break;
			}
			
			// Try to start
			if (!sim7080g_is_ready(_gateway) || !sim7080g_config(_gateway))
				break;

			MODEM_CORE_STATE = MODEM_STARTED;
			break;

		case MODEM_STARTED:
			if (!sim7080g_is_ready(_gateway) || cbuffer_empty(_modem_buffer_out)) {
				MODEM_CORE_STATE = MODEM_STOPPED;
				break;
			}

			if (!_modem_cn_available())
				break;

			if (!sim7080g_cn_activate(_gateway, true)) {
				sim7080g_cn_activate(_gateway, false);
				break;
			}

			MODEM_CORE_STATE = MODEM_CN_ACTIVE;
			break;

		case MODEM_CN_ACTIVE:
			if (!sim7080g_cn_is_active(_gateway) || cbuffer_empty(_modem_buffer_out)) {
				sim7080g_cn_activate(_gateway, false);
				MODEM_CORE_STATE = MODEM_STARTED;
				break;
			}

			if (!sim7080g_tcp_open(_gateway, strlen(SERVER_IP), SERVER_IP, 8086)) {
				sim7080g_tcp_close(_gateway);
				break;
			}

			//sim7080g_ssl_enable(_gateway,false);

			MODEM_CORE_STATE = MODEM_SERVER_CONNECTED;
			break;

		case MODEM_SERVER_CONNECTED:
			if (!sim7080g_tcp_is_open(_gateway) || ((cbuffer_empty(_modem_buffer_out) && !_modem_send_incomplete))) {
				MODEM_CORE_STATE = MODEM_CN_ACTIVE;
				sim7080g_tcp_close(_gateway);
				break;
			}

			_modem_send_incomplete = !_modem_buffer_send();
			
			// Wait for data to be acknowleged
			uint sent = 0;
			uint unack = 0;
			while (sim7080g_tcp_is_open(_gateway)) {
				sim7080g_tcp_ack(_gateway, &sent, &unack);
				if (sent == 0 || unack == 0) break;
				sleep_ms(500);
			}

			break;
		}

		//printf("state: %u\n", MODEM_CORE_STATE);

LOOP_CONTINUE:;
		sleep_ms(1000);
	}
}

static bool _modem_cn_available(void) {
	if (MODEM_CORE_STATE <= MODEM_STOPPED)
		return false;

	return sim7080g_cn_available(_gateway);
}

#define GW_Q_BUF_SIZE 400
#define GW_Q_BUF_MALLOC_FAILED -6
static int _message_queue_process(void) {
	uint32_t buffer = 0;
	uint16_t received = 0;
	uint16_t new_received;

	// Read until return is 0 or < 0 (indicating error)
	while (new_received = gw_queue_gw_recv((uint8_t *)&buffer, (sizeof buffer))) {

		//printf("received: %u\n", new_received);

		if (new_received < 0) gw_core_panic(GW_CORE_FAILURE);

		received += new_received;
		// dispatch each uint32_t read to message buffer
		_dispatch_message_buffer(buffer);
	}
	
	return received;
}

static void _dispatch_message_buffer(uint32_t buffer) {
	static bool new_message = true;
	static uint32_t message_type;

	//printf("nm: %s\n", new_message ? "true" : "false");
	//printf("buffer: %u\n", buffer);

	if (new_message) {
		message_type = buffer;
		new_message = false;
		return;
	} 
	uint rval = 0;
	switch(message_type) {
	case GATEWAY_COMMAND:
		rval = _build_command(buffer);
		// Unrecoverable error if rval < 0
		if (rval < 0) gw_core_panic(GW_CORE_FAILURE);
		if (rval == COMMAND_DISPATCHED) new_message = true;
		break;
	case PACKED_DATA:
		rval = _build_packed(buffer);
		if (rval < 0) gw_core_panic(GW_CORE_FAILURE);
		if (rval == PACKED_COMPLETE) new_message = true;
		break;
	}
}

static int _build_command(uint32_t buffer) {
	static bool new_command = true;
	static uint32_t command_type;

	if (new_command) {
		command_type = buffer;
		//printf("ct: %u\n", command_type);
		new_command = false;
	}

	uint rval = COMMAND_UNRECOGNIZED;

	switch (command_type) {
	case GATEWAY_START:
    case GATEWAY_STOP:
		if (_command_buffer_push(&buffer, (sizeof buffer))) {
			//printf("cb pushed\n");
			rval = COMMAND_DISPATCHED;
		}
		else rval = COMMAND_BUFFER_FULL;
		break;
	}

	// TODD: Need some better way to deal with full command buffer. How to reissue command or tell
	// main core that command failed?
	// 10.10.24 We've gone single core. It is all fucked now.
	if (rval == COMMAND_DISPATCHED) new_command = true;

	return rval;
}

static int _build_packed(uint32_t buffer) {
	static bool new_packed = true;
	static uint32_t packed_size;
	static uint buffer_size = sizeof buffer;

	if (new_packed) {
		packed_size = buffer;
		//printf("packed_size: %u\n", packed_size);
		new_packed = false;
		return PACKED_INCOMPLETE;
	}

	uint push_size = buffer_size < packed_size ? buffer_size : packed_size;
	if(_modem_buffer_push(&buffer, push_size)) {
		//printf("pb pushed: %u\n", push_size);
		packed_size -= push_size;
		//printf("packed_size remaining: %u\n", packed_size);
	}

	if (!packed_size) {
		new_packed = true;
		return PACKED_COMPLETE;
	}

	return PACKED_INCOMPLETE;
}

static bool _command_buffer_push(void *command, uint size) {
	//printf("cbr: %u | size: %u\n", cbuffer_remaining(_modem_buffer_command), size);
	if (size > cbuffer_remaining(_modem_buffer_command)) return false;	

	cbuffer_push(_modem_buffer_command, command, size);

	return true;
}

static bool _modem_buffer_push(void *buffer, uint size) {
	//printf("pbl: %u | size: %u\n", cbuffer_length(_modem_buffer_out), size);
	if (size > cbuffer_remaining(_modem_buffer_out)) return false;	

	cbuffer_push(_modem_buffer_out, buffer, size);

	return true;
}

static void _command_buffer_execute(void) {
	static bool new_command = true;
	static uint32_t command_type;

	while (!cbuffer_empty(_modem_buffer_command)) {
		if (new_command) {
			cbuffer_pop(_modem_buffer_command, &command_type, (sizeof command_type));
			new_command = false;
		}

		switch (command_type) {
		case GATEWAY_START:
			_start_command_issued = true;
			new_command = true;
			break;
		case GATEWAY_STOP:
			_start_command_issued = false;
			_modem_power_toggled = false;
			_modem_power_down = true;
			new_command = true;
			break;	
		}
	}
}

static bool _modem_buffer_send(void) {
	static uint8_t data[20];
	static int popped;
	static bool data_buffered = false;

	while (!cbuffer_empty(_modem_buffer_out) || data_buffered) {
		if (!data_buffered) {
			popped = cbuffer_pop(_modem_buffer_out, data, sizeof data);
			//printf("popped: %u\n", popped);
			//printf("cblen: %u\n", cbuffer_length(_modem_buffer_out));
			data_buffered = true;
		}

		if (!sim7080g_tcp_send(_gateway, popped, data))
			break;

		data_buffered = false;
		//printf("data: %X\n", data);
	}
	
	return !data_buffered;
}
