#include <string.h>
#include <stdio.h>
#include "pico/time.h"
#include "hardware/gpio.h"

#include "modem.h"

#define MODEM_START_RETRIES 20

struct _modem {
	uart_inst_t *uart;
	uint pin_tx;
	uint pin_rx;
	uint pin_power;
};

static Modem MODEM;
static bool MODEM_STARTED = false;

Modem *modem_start(
	char *apn,
	uart_inst_t *uart,	
	uint pin_tx,
	uint pin_rx,
	uint pin_power
)
{
	// This protects us from wierd things happening
	// if we call this function twice
	if (MODEM_STARTED) return &MODEM;

	MODEM.uart = uart;
	MODEM.pin_tx = pin_tx;
	MODEM.pin_rx = pin_rx;
	MODEM.pin_power = pin_power;

	char buf[256];
	bool reset = false;
	for (int i = 0; i < MODEM_START_RETRIES; i++) {

		bool success = modem_at_send(&MODEM, "ATE1", "OK", buf, 256, 100);
		if (success) {
			MODEM_STARTED = true;

			printf("%s\n", buf);

			success = modem_config(&MODEM, apn);

			return &MODEM; // Successful return
		}

		if (!reset) {
			modem_toggle_power(&MODEM);
			reset = true;
			sleep_ms(4500);
		}
		
		sleep_ms(500);
	}

	return NULL;
}

bool modem_at_send(
		Modem *modem, 
		char *command, 
		char *needle, 
		char *return_buffer,
		uint buffer_size,
		uint timeout
)
{
	uart_puts(modem->uart, command);
	uart_puts(modem->uart, "\r\n");

	char *p = return_buffer;

	absolute_time_t now = get_absolute_time();
	while ((get_absolute_time() - now < timeout * 1000) 
		   && (p - return_buffer < buffer_size))
	{
		while (uart_is_readable_within_us(modem->uart, 2000) > 0)
			uart_read_blocking(modem->uart, p++, 1);
	}
	*p = '\0';

	// If we hit the end of our buffer
	if ((p - return_buffer) > buffer_size) return false;

	// Return if expected substring was found in return
	return !!strstr(return_buffer, needle);
}

static bool modem_config(Modem *modem, char *apn) {
	char buf[256];
	// +CMEE=2  Verbose errors
	// +CMGF=1  SMS message format: text
	// +CMGD=4  Clear any existing SMS messages in buffer
	// +CNMP=38 Preferred mode: LTE only
	// +CMNB=1  Preferred network: CAT-M
	bool success = modem_at_send(
			modem,
			"AT+CMEE=2;+CMGF=1;+CMGD=4;+CNMP=38;+CMNB=1;",
			"OK",
			buf,
			256,
			500
	);
	if (!success) return false;
	printf("%s\n", buf);

	// Define PDP context
	char command[256] = "AT+CGDCONT=1,\"IP\",\"";
	strcat(command, apn);
	strcat(command, "\"");

	success = modem_at_send(
			modem,
			command,
			"OK",
			buf,
			256,
			500
	);
	if (!success) return false;
	printf("%s\n", buf);

	return success;
}


bool modem_toggle_power(Modem *modem) {
	gpio_put(modem->pin_power, 1);
	sleep_ms(2500);
	gpio_put(modem->pin_power, 0);
}
