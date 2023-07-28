#ifndef WISDOM_MODEM_H
#define WISDOM_MODEM_H
#include <stdbool.h>
#include <stdint.h>

#include "hardware/uart.h"

#include "modem_core.h"
#include "command_buffer.h"
#include "response_parser.h"

#define MODEM_READ_BUFFER_SIZE 1024
#define WRITE_TIMEOUT_RESOLUTION_US 100
#define READ_STOP_TIMEOUT_US (1000 * 10)

// Modem state object
typedef struct _modem {
	uart_inst_t *uart;
	uint pin_tx;
	uint pin_rx;
	uint pin_power;
} Modem;

// Powers on and configures modem
//
// apn		 - sim provider APN string
// uart		 - UART hardware instance
// pin_tx	 - UART tx gpio pin
// pin_rx    - UART rx bpio pin
// pin_power - modem power gpio pin
//
// return: pointer to Modem state object singleton if modem started successfully
// 		   NULL pointer returned otherwise
Modem *modem_start(
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
void modem_write_blocking(
		Modem modem[static 1],
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
bool modem_write_within_us (
		Modem *modem, 
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
uint32_t modem_read_blocking(Modem modem[static 1], uint8_t *dst, size_t dst_len);

// Reads from moding and checks if data contains OK message
//
// modem - Modem state object pointer
//
// return: true if read data contains OK message
bool modem_read_blocking_ok(Modem modem[static 1]);

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
uint32_t modem_read_within_us(
		Modem *modem, 
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
void modem_cb_write_blocking(Modem modem[static 1], CommandBuffer cb[static 1]);

// Write to modem from CommandBuffer 
//
// modem - pointer to Modem state object
// cb    - pointer to source CommandBuffer
// timeout - Timout time in micro-seconds
//
// NON-BLOCKING: Will not block for > (timeout + WRITE_TIMEOUT_RESOLUTION_US)
bool modem_cb_write_within_us(
		Modem *modem, 
		CommandBuffer *cb,
		uint64_t timeout	
);


// Tests if modem is ready to accept commands
//
// modem - pointer to Modem state object
//
// return: true if modem is responsive
//         false otherwise
bool modem_is_ready(Modem modem[static 1]);

// Tests if sim card is ready
//
// modem - pointer to Modem state object
//
// return: true if sim card status = READY
//         false otherwise
bool modem_sim_ready(Modem modem[static 1]);

// Tests if a network is available 
//
// modem - pointer to Modem state object
//
// return: true if a network is detected
// 		   false otherwise
bool modem_cn_available(Modem modem[static 1]);

// Tests if a network connection is currently active
//
// modem - pointer to Modem state object
//
// return: true if a network connection is active
bool modem_cn_is_active(Modem modem[static 1]);

// Activate/deactivate network connection
//
// modem    - pointer to Modem state object
// activate - true to activate
// 			  false to deactivate
//
// return: true if command was successful
//         false if there was an error
bool modem_cn_activate(Modem modem[static 1], bool activate);

// Enable/disable SSL
//
// modem  - pointer to Modem state object
// enable - true to enable
// 			false to disable
bool modem_ssl_enable(Modem modem[static 1], bool enable);

// Block until a network is available
//
// modem  - pointer to Modem state object
void modem_wait_for_cn(Modem modem[static 1]);

// Opens a TCP connection with a remote server
//
// modem   - pointer to Modem state object
// url_len - length of remote server URL/IP address string
// url     - remote server URL/IP string
// port    - remote server port
//
// return: true if command was successful
//         false if there was an error
bool modem_tcp_open(
		Modem modem[static 1], 
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
bool modem_tcp_close(Modem modem[static 1]);

bool modem_tcp_is_open(Modem modem[static 1]);

void modem_read_to_null(Modem modem[static 1]);

bool modem_toggle_power(Modem *modem);

#endif // WISDOM_MODEM_H
