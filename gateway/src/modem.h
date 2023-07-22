#ifndef WISDOM_MODEM_H
#define WISDOM_MODEM_H
#include <stdbool.h>
#include <stdint.h>

#include "hardware/uart.h"

#include "modem_core.h"
#include "command_buffer.h"
#include "response_parser.h"

typedef struct _modem Modem;

bool modem_write_within_us (
		Modem *modem, 
		const uint8_t *src, 
		size_t src_len, 
		uint64_t us
); 

uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len, 
		uint64_t us
); 

bool modem_command_write_within_us(
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

static bool modem_config(Modem *modem, char *apn);

bool modem_toggle_power(Modem *modem);

#endif // WISDOM_MODEM_H
