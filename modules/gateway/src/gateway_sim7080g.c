#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "sim7080g_pico.h"
#include "cbuffer.h"

#include "gateway_interface.h"
#include "gateway_error.h"

#include "gateway_codes.h" // Communication codes
						   
#define UART_BAUD 115200

#define SERVER_IP "73.149.88.183"
						   
enum _modem_state_e {
	MODEM_UNINITIALIZED,
	MODEM_POWERED_DOWN,
	MODEM_STOPPED,
	MODEM_STARTED,
	MODEM_CN_ACTIVE,
	MODEM_SERVER_CONNECTED
};
static sim7080g_context_t *_gateway = NULL;

// Yup, a global for exposing state. Don't yell at me.
int MODEM_CORE_STATE = MODEM_UNINITIALIZED;

static bool _modem_power_down = false;
static bool _modem_power_toggled = false;
static bool _modem_ssl_configured = false;
static bool _modem_send_incomplete = false;

// Command flags
//static bool _start_command_issued = false;

// Data output buffer
#define MODEM_BUFFER_OUT_SIZE (1024 * 10) // 10 KB
static cbuffer_t _modem_buffer_out_base;
static cbuffer_t *_modem_buffer_out = &_modem_buffer_out_base;
uint8_t _modem_buffer_array[MODEM_BUFFER_OUT_SIZE];

// Command buffer
// Should only need to hold several commands at a time
#define MODEM_BUFFER_COMMAND_SIZE (sizeof (uint32_t) * 100)
//static cbuffer_t *_modem_buffer_command = NULL; // Internal func prototypes
static bool _modem_cn_available(void);
//static int _message_queue_process(void);
//static void _dispatch_message_buffer(uint32_t buffer);
//static int _build_command(uint32_t buffer);
//static int _build_packed(uint32_t buffer);
//static bool _command_buffer_push(void *command, uint size);
static bool _modem_buffer_push(void *buffer, uint size);
//static void _command_buffer_execute(void);
static bool _modem_buffer_send(void);

bool gateway_init(void) {
	bool success = false;
	cbuffer_init(_modem_buffer_out, _modem_buffer_array, MODEM_BUFFER_OUT_SIZE);

	if (!_modem_buffer_out) // || !_modem_buffer_command)
		goto RETURN_SUCCESS;

	_gateway = sim7080g_create();
	if (_gateway == NULL)
		goto RETURN_SUCCESS;

	sim7080g_init(
			_gateway,
			GATEWAY_APN,
		
			GATEWAY_UART,
			GATEWAY_PIN_TX,
			GATEWAY_PIN_RX,
			GATEWAY_PIN_PWR
	);

	MODEM_CORE_STATE = MODEM_POWERED_DOWN;

	success = true;
RETURN_SUCCESS:
	return success;

// OLD
	//gw_queue_init();

	//multicore_launch_core1(gw_core_entry);
}

int gateway_pump(void) {

	//_message_queue_process();
	//_command_buffer_execute();

	static int count = 0;
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

		if (count == 100) {
			sim7080g_toggle_power(_gateway);
			count = 0;
			break;
		}
		count++;

		
		// Try to start
		if (!sim7080g_is_ready(_gateway))
			break;

		sim7080g_config(_gateway);

		count = 0;
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

	return MODEM_CORE_STATE;
}

int gateway_state_get(void) {
	return MODEM_CORE_STATE;
}

int gateway_queue_push (void *data, uint32_t size) {
	return cbuffer_push(_modem_buffer_out, (uint8_t *)data, size);
}

bool gateway_recv(void *data, uint size) {

	return 0;
}

static bool _modem_cn_available(void) {
	if (MODEM_CORE_STATE <= MODEM_STOPPED)
		return false;

	return sim7080g_cn_available(_gateway);
}

static bool _modem_buffer_send(void) {
	static uint8_t data[100];
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
