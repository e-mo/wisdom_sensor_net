#ifndef SIM7080G_RP2X_LIB_H
#define SIM7080G_RP2X_LIB_H

#include "at_definitions.h"
#include "cbuffer.h"

typedef unsigned uint;

struct sim7080g_config {
	uint uart_index;
	uint pin_pwr;
};

typedef struct sim7080g_context {
	uint uart_index;
	uint pin_pwr;

} sim7080g_context_t;

void sim7080g_init(sim7080g_context_t *context, const struct sim7080g_config config);

void sim7080g_pwr_toggle(sim7080g_context_t *context);

uint sim7080g_uart_read(sim7080g_context_t *context, void *buffer, size_t size);
uint sim7080g_uart_write(sim7080g_context_t *context, const void *buffer, size_t size);
void sim7080g_uart_write_all(sim7080g_context_t *context, const void *buffer, size_t size);

void sim7080g_cn_state(sim7080g_context_t *context);
void sim7080g_cn_activate(sim7080g_context_t *context, uint cn_index, bool activate);

#endif // SIM7080G_RP2X_LIB_H
