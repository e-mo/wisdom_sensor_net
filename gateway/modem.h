#ifndef WISDOM_MODEM_H
#define WISDOM_MODEM_H
#include <stdbool.h>
#include <stdint.h>

#include "hardware/uart.h"

#include "modem_core.h"
#include "command_buffer.h"

typedef struct _modem Modem;

bool modem_write_within_us (
		Modem *modem, 
		const uint8_t *src, 
		size_t src_len, 
		uint32_t us
); 

uint32_t modem_read_within_us(
		Modem *modem, 
		uint8_t *dst, 
		size_t dst_len, 
		uint32_t us
); 

bool modem_command_write(Modem *modem, CommandBuffer *cb);

Modem *modem_start(
		char *apn,
		uart_inst_t *uart,	
		uint pin_tx,
		uint pin_rx,
		uint pin_power
);

static bool modem_config(Modem *modem, char *apn);

bool modem_toggle_power(Modem *modem);
static void modem_gpio_init(Modem *modem);

#endif // WISDOM_MODEM_H
