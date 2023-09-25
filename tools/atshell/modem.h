#ifndef WISDOM_MODEM_H
#define WISDOM_MODEM_H

#include "hardware/uart.h"

typedef struct _modem Modem;

Modem *modem_start(
		char *apn,
		uart_inst_t *uart,	
		uint pin_tx,
		uint pin_rx,
		uint pin_power
);

bool modem_at_send(
		Modem *modem,
		char *command,
		char *needle,
		char *return_buffer,
		uint buffer_size,
		uint timeout
); 

static bool modem_config(Modem *modem, char *apn);

bool modem_toggle_power(Modem *modem);
static void modem_gpio_init(Modem *modem);

#endif // WISDOM_MODEM_H
