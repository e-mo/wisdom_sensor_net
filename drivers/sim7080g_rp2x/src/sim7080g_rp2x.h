#ifndef SIM7080G_RP2X_LIB_H
#define SIM7080G_RP2X_LIB_H

#include "at_definitions.h"
#include "cbuffer.h"

typedef unsigned uint;

typedef enum {
	MODEM_CN_INACTIVE,
	MODEM_CN_ACTIVE
} SIM7080G_CN_STATE;

struct sim7080g_config {
	uint uart_index;
	uint pin_pwr;
};

typedef enum {
	ATR_SEARCHING,
	ATR_CONFIRMING,
	ATR_PARSING
} ATR_PARSING_STATE;

typedef enum {
	AT_RESPONSE_CLEAR,
	AT_RESPONSE_OK,
	AT_RESPONSE_ERROR,
	AT_RESPONSE_CME_ERROR
} AT_RESPONSE_STATE;

#define MODEM_CN_MAX (4)
typedef struct sim7080g_context {
	uint uart_index;
	uint pin_pwr;

	//SIM7080G_CN_STATE cn_state[MODEM_CN_MAX];

	//// Internal parsing state
	//ATR_PARSING_STATE parsing_state;
	//size_t match_index;
	//size_t needle_index;

	//uint8_t parse_buffer[20];
	//size_t parse_index;

	//AT_RESPONSE_STATE response_state;
} sim7080g_context_t;

void sim7080g_init(sim7080g_context_t *context, const struct sim7080g_config config);

void sim7080g_pwr_toggle(sim7080g_context_t *context);

uint sim7080g_uart_read(sim7080g_context_t *context, void *buffer, size_t size);
uint sim7080g_uart_write(sim7080g_context_t *context, const void *buffer, size_t size);
void sim7080g_uart_write_all(sim7080g_context_t *context, const void *buffer, size_t size);

void sim7080g_cn_state(sim7080g_context_t *context);
void sim7080g_cn_activate(sim7080g_context_t *context, uint cn_index, bool activate);

#endif // SIM7080G_RP2X_LIB_H
