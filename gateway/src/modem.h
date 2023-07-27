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

typedef struct _modem {
	uart_inst_t *uart;
	uint pin_tx;
	uint pin_rx;
	uint pin_power;
} Modem;

// Writes to modem over UART
//
// modem   - Modem state object pointer 
// src	   - Source buffer to write to modem
// src_len - Length of source buffer
//
// BLOCKING - will block until entire source buffer is written to UART
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
// us      - Timout time in micro-seconds
//
// NON-BLOCKING - Will not block longer than:
//               (us + WRITE_TIMEOUT_RESOLUTION_US)
bool modem_write_within_us (
		Modem *modem, 
		const uint8_t *src, 
		size_t src_len, 
		uint64_t us
); 

uint32_t modem_read_blocking(Modem *modem, uint8_t *dst, size_t dst_len);
bool modem_read_blocking_ok(Modem modem[static 1], uint8_t dst[], size_t dst_len);

uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len,
		uint64_t us
); 

void modem_cb_write_blocking(Modem modem[static 1], CommandBuffer cb[static 1]);

bool modem_cb_write_within_us(
		Modem *modem, 
		CommandBuffer *cb,
		uint64_t us		
);

Modem *modem_start(
		char *apn,
		uart_inst_t *uart,	
		uint pin_tx,
		uint pin_rx,
		uint pin_power
);

bool modem_is_ready(Modem modem[static 1]);
bool modem_sim_ready(Modem modem[static 1]);
static bool modem_config(Modem *modem, char *apn);
bool modem_cn_ready(Modem modem[static 1]);
void modem_wait_for_network(Modem modem[static 1]);

bool modem_toggle_power(Modem *modem);

#endif // WISDOM_MODEM_H
