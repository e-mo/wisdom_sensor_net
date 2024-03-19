#ifndef WISDOM_MODEM_H
#define WISDOM_MODEM_H
#include <stdbool.h>
#include <stdint.h>

#include "hardware/uart.h"

#include "command_buffer.h"
#include "response_parser.h"

#define MODEM_READ_BUFFER_SIZE 1024
#define WRITE_TIMEOUT_RESOLUTION_US 100
#define READ_STOP_TIMEOUT_US (1000 * 10)
#define MODEM_TCP_SEND_MAX 1459

#define UART_PORT uart0
#define UART_BAUD 115200
#define UART_PIN_TX 0
#define UART_PIN_RX 1

#define MODEM_PIN_PWR 14
#define MODEM_APN "iot.1nce.net"
#define SERVER_URL "34.27.138.203"
#define SERVER_PORT 8086

// Modem state object
typedef struct _sim7080g_context {
	uart_inst_t *uart;
	uint pin_tx;
	uint pin_rx;
	uint pin_power;
} sim7080g_context_t;

// Powers on and configures modem
//
// apn		 - sim provider APN string
// uart		 - UART hardware instance
// pin_tx	 - UART tx gpio ping
// pin_rx    - UART rx bpio pin
// pin_power - modem power gpio pin
//
// return: pointer to Modem state object singleton if modem started successfully
// 		   NULL pointer returned otherwise
sim7080g_context_t *sim7080g_start(
		char *apn,
		uart_inst_t *uart,	
		uint pin_tx,
		uint pin_rx,
		uint pin_power
);

// Writes to modem over UART
//
// modem   - Modem state object pointer 
// src	   - Source buffer to write to modem
// src_len - Length of source buffer
//
// BLOCKING: will block until entire source buffer is written to UART
void sim7080g_write_blocking(
		sim7080g_context_t *context,
		const uint8_t src[],
		size_t src_len
);

// Writes to modem over UART with a timeout
//
// modem   - Modem state object pointer 
// src	   - Source buffer to write to modem
// src_len - Length of source buffer
// timeout - Timout time in micro-seconds
//
// return: true if source buffer was written to uart
// 		   false if operation times out
//
// NON-BLOCKING: Will not block for > (timeout + WRITE_TIMEOUT_RESOLUTION_US)
bool sim7080g_write_within_us (
		sim7080g_context_t *context, 
		const uint8_t *src, 
		size_t src_len, 
		uint64_t timeout
); 

// Reads from modem into buffer
//
// modem   - Modem state object pointer
// dst     - Destination buffer to read into
// dst_len - Length of destination buffer
//
// return: # of bytes read to buffer
//
// BLOCKING: will block until data is received from modem
//
// Under the hood this function works by blocking on a UART read until
// data is available. Once data has been received, data will continue
// to be read into the buffer until the buffer is full, or until new
// data has not been received in > READ_STOP_TIMEOUT_US
uint32_t sim7080g_read_blocking(sim7080g_context_t *context, uint8_t *dst, size_t dst_len);

// Reads from moding and checks if data contains OK message
//
// modem - Modem state object pointer
//
// return: true if read data contains OK message
bool sim7080g_read_blocking_ok(sim7080g_context_t *context);

bool sim7080g_read_ok_within_us(sim7080g_context_t *context, uint64_t timeout);

// Reads from modem into buffer
//
// modem   - Modem state object pointer
// dst     - Destination buffer to read into
// dst_len - Length of destination buffer
// timeout - Timout time in micro-seconds
//
// return: # of bytes read to buffer
//
// NON-BLOCKING: Will not block for > (timout + READ_STOP_TIMOUT_US) 
uint32_t sim7080g_read_within_us(
		sim7080g_context_t *context, 
		uint8_t *dst, 
		size_t dst_len,
		uint64_t timeout
); 

// Write to modem from CommandBuffer 
//
// modem - pointer to Modem state object
// cb    - pointer to source CommandBuffer
//
// return: # of bytes read to buffer
//
// BLOCKING: will block until entire CommandBuffer is written to UART
void sim7080g_cb_write_blocking(sim7080g_context_t *context, CommandBuffer cb[static 1]);

// Write to modem from CommandBuffer 
//
// modem - pointer to Modem state object
// cb    - pointer to source CommandBuffer
// timeout - Timout time in micro-seconds
//
// NON-BLOCKING: Will not block for > (timeout + WRITE_TIMEOUT_RESOLUTION_US)
bool sim7080g_cb_write_within_us(
		sim7080g_context_t *context, 
		CommandBuffer *cb,
		uint64_t timeout	
);


// Tests if modem is ready to accept commands
//
// modem - pointer to Modem state object
//
// return: true if modem is responsive
//         false otherwise
bool sim7080g_is_ready(sim7080g_context_t *context);

// Tests if sim card is ready
//
// modem - pointer to Modem state object
//
// return: true if sim card status = READY
//         false otherwise
bool sim7080g_sim_ready(sim7080g_context_t *context);

// Tests if a network is available 
//
// modem - pointer to Modem state object
//
// return: true if a network is detected
// 		   false otherwise
bool sim7080g_cn_available(sim7080g_context_t *context);

// Tests if a network connection is currently active
//
// modem - pointer to Modem state object
//
// return: true if a network connection is active
bool sim7080g_cn_is_active(sim7080g_context_t *context);

// Activate/deactivate network connection
//
// modem    - pointer to Modem state object
// activate - true to activate
// 			  false to deactivate
//
// return: true if command was successful
//         false if there was an error
bool sim7080g_cn_activate(sim7080g_context_t *context, bool activate);

// Enable/disable SSL
//
// modem  - pointer to Modem state object
// enable - true to enable
// 			false to disable
bool sim7080g_ssl_enable(sim7080g_context_t *context, bool enable);

// Block until a network is available
//
// modem  - pointer to Modem state object
void sim7080g_wait_for_cn(sim7080g_context_t *context);

// Opens a TCP connection with a remote server
//
// modem   - pointer to Modem state object
// url_len - length of remote server URL/IP address string
// url     - remote server URL/IP string
// port    - remote server port
//
// return: true if command was successful
//         false if there was an error
bool sim7080g_tcp_open(
		sim7080g_context_t *context, 
		uint8_t url_len,  
		uint8_t url[static url_len],
		uint16_t port
); 

// Close a TCP connection with a remote server
//
// modem   - pointer to Modem state object
//
// return: true if command was successful
//         false if there was an error
bool sim7080g_tcp_close(sim7080g_context_t *context);


bool sim7080g_tcp_send(
		sim7080g_context_t *context,
		size_t data_len,
		uint8_t data[static data_len]
);

size_t sim7080g_tcp_recv(
		sim7080g_context_t *context,
		size_t dst_len,
		uint8_t dst[dst_len]
);

size_t sim7080g_tcp_recv_within_us(
		sim7080g_context_t *context,
		size_t dst_len,
		uint8_t dst[dst_len],
		uint64_t timeout
);

bool sim7080g_tcp_recv_ready_within_us(sim7080g_context_t *context, uint64_t timeout);

bool sim7080g_tcp_is_open(sim7080g_context_t *context);

void sim7080g_read_to_null(sim7080g_context_t *context);

bool sim7080g_toggle_power(sim7080g_context_t *context);

bool sim7080g_power_down(sim7080g_context_t *context);

#endif // WISDOM_MODEM_H
