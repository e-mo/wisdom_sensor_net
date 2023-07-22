#ifndef WISDOM_MODEM_COMMAND_BUFFER_H
#define WISDOM_MODEM_COMMAND_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define COMMAND_BUFFER_MAX 559 // per SIM7080_Series_AT_Command_Manual_V1.02.pdf

typedef struct _command_buffer CommandBuffer;

CommandBuffer *cb_create();
void cb_destroy(CommandBuffer *cb);
uint32_t cb_write(CommandBuffer *cb, uint8_t *src, uint32_t src_len);

uint32_t cb_length(CommandBuffer *cb);
bool cb_full(CommandBuffer *cb);
bool cb_empty(CommandBuffer *cb);
uint8_t *cb_get(CommandBuffer *cb);
void cb_clear(CommandBuffer *cb);
bool cb_prefix_set(CommandBuffer *cb);

#endif // WISDOM_MODEM_COMMAND_BUFFER_H
