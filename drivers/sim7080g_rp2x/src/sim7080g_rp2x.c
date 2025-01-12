#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "sim7080g_rp2x.h"

void sim7080g_init(sim7080g_context_t *context, const struct sim7080g_config config) {
	context->uart_index = config.uart_index;
	context->pin_pwr = config.pin_pwr;

	gpio_init(context->pin_pwr);
	gpio_set_dir(context->pin_pwr, GPIO_OUT);
	gpio_put(context->pin_pwr, 1);

	//memset(context->cn_state, MODEM_CN_INACTIVE, MODEM_CN_MAX);
	//context->cn_state;

	//context->parsing_state = ATR_SEARCHING;	
	//context->needle_index = 1;
	//context->parse_index = 0;

	//context->response_state = AT_RESPONSE_CLEAR;
}

//AT_RESPONSE_STATE sim7080g_response_state(sim7080g_context_t *context) {
//	return context->response_state;
//}

//void sim7080g_response_clear

static bool _pwr_switch(sim7080g_context_t *context) {
	bool level = gpio_get(context->pin_pwr);
	gpio_put(context->pin_pwr, !level);
	return level;
}

void sim7080g_pwr_toggle(sim7080g_context_t *context) {
	_pwr_switch(context);
	sleep_ms(1100);
	_pwr_switch(context);
}

void sim7080g_pwr_set(sim7080g_context_t *context, bool level) {
	gpio_put(context->pin_pwr, level);
}

//static void _atr_app_pdp_parse(sim7080g_context_t *context) {
//	uint8_t *buffer = context->parse_buffer;
//	buffer++; // Skip space
//	buffer[1] = '\0';	
//	uint cn_index = atoi(buffer);
//	fflush(stdout);
//	const char state = buffer[2];
//	switch (state) {
//	case 'A':
//		context->cn_state[cn_index] = MODEM_CN_ACTIVE; 
//		break;
//	case 'D':
//		context->cn_state[cn_index] = MODEM_CN_INACTIVE; 
//		break;
//	}
//}
//
//static void _parse_response(sim7080g_context_t *context, uint8_t c) {
//	switch (context->parsing_state) {
//	case ATR_SEARCHING:
//		for (int i = ATR_OK; i < AT_STRING_COUNT; i++)
//			if (c == at_string_lookup[i][0]) {
//				context->parsing_state = ATR_CONFIRMING;
//				context->match_index = i;
//				break;
//			}
//
//		printf("searching\n");
//		break;
//	case ATR_CONFIRMING:
//		// If not still matching
//		printf("confirming\n");
//		if (c != at_string_lookup[context->match_index][context->needle_index]) {
//			// Reset needle index
//			context->needle_index = 1;
//			context->parsing_state = ATR_SEARCHING;
//			break;
//		}
//
//		context->needle_index++;
//
//		// If we have reached the null, we have a match.
//		if (at_string_lookup[context->match_index][context->needle_index] == '\0') {
//			context->parsing_state = ATR_PARSING;
//		}
//
//		break;
//	case ATR_PARSING:
//	
//		switch (context->match_index) {
//		case ATR_OK:
//			printf("OKAYYYYY!\n");
//			context->response_state = AT_RESPONSE_OK;
//
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_ERROR:
//			context->response_state = AT_RESPONSE_ERROR;
//
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_CME_ERROR:
//			context->response_state = AT_RESPONSE_CME_ERROR;
//
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_APP_PDP:
//			if (c != '\r') {
//				context->parse_buffer[context->parse_index++] = c;
//				break;
//			}
//
//			printf("Here!\n");
//
//			_atr_app_pdp_parse(context);
//
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_CAOPEN:
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_CACLOSE:
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_CASTATE:
//			goto PARSE_CLEANUP;
//			break;
//		case ATR_NPD:
//			// Set all CN to inactive
//			memset(context->cn_state, MODEM_CN_INACTIVE, MODEM_CN_MAX);
//			goto PARSE_CLEANUP;
//			break;
//		}
//
//		break;
//
//PARSE_CLEANUP:
//		context->parse_index = 0; // Reset parse index
//		context->parsing_state = ATR_SEARCHING;
//		break;
//	}
//
//}

uint sim7080g_uart_read(sim7080g_context_t *context, void *buffer, size_t size) {
	uint count = 0;
	uint8_t c;
	while(uart_is_readable(UART_INSTANCE(context->uart_index)) && count < size) {
		uart_read_blocking(UART_INSTANCE(context->uart_index), &c, 1);	

		// Sometimes garbage bytes come through when uart starts up
		// so we have to filter those out.
		// However, we still want to pass on \r and \n so we check
		// for those.
		if (!isprint(c))
			if (c != '\r' && c != '\n')	
				continue;

		if (buffer != NULL)
			*(char *)buffer++ = c;

		// Internally parse all responses to keep track of state
		//_parse_response(context, c);
		count++;
	}

	return count;
}

uint sim7080g_uart_write(sim7080g_context_t *context, const void *buffer, size_t size) {
	uint count = 0;
	while(uart_is_writable(UART_INSTANCE(context->uart_index)) && count < size) {
		uart_write_blocking(UART_INSTANCE(context->uart_index), buffer++, 1);	
		count++;
	}

	return count;
}

void sim7080g_uart_write_all(sim7080g_context_t *context, const void *buffer, size_t size) {
	uint count = 0;
	while (size) {
		count = sim7080g_uart_write(context, buffer, size);
		size -= count;
		buffer = (uint8_t *)buffer + count;
	}
}

void sim7080g_cn_state(sim7080g_context_t *context) {
	const char *str = at_string_lookup[AT_PREFIX];
	sim7080g_uart_write(context, str, strlen(str));		

	str = at_string_lookup[ATC_CNACT];
	sim7080g_uart_write(context, str, strlen(str));		

	sim7080g_uart_write(context, "?\r", 2);		
}

void sim7080g_cn_activate(sim7080g_context_t *context, uint cn_index, bool activate) {
	const char *str = at_string_lookup[AT_PREFIX];
	sim7080g_uart_write(context, str, strlen(str));		

	str = at_string_lookup[ATC_CNACT];
	sim7080g_uart_write(context, str, strlen(str));		

	char format[6];
	sprintf(format, "=%u,%c\r", cn_index, activate ? '1' : '0');
	sim7080g_uart_write(context, format, 5);		
}
