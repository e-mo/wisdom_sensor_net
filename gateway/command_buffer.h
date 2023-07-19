#ifndef WISDOM_MODEM_COMMAND_BUFFER_H
#define WISDOM_MODEM_COMMAND_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define COMMAND_BUFFER_MAX 559 // per SIM7080_Series_AT_Command_Manual_V1.02.pdf

typedef struct _command_buffer CommandBuffer;

CommandBuffer *command_buffer_create();
void command_buffer_destroy(CommandBuffer *cb);
uint32_t command_buffer_write(CommandBuffer *cb, uint8_t *src, uint32_t src_len);

uint32_t command_buffer_length(CommandBuffer *cb);
bool command_buffer_full(CommandBuffer *cb);
bool command_buffer_empty(CommandBuffer *cb);
uint8_t *command_buffer_get(CommandBuffer *cb);
void command_buffer_clear(CommandBuffer *cb);
bool command_buffer_prefix_set(CommandBuffer *cb);

#endif // WISDOM_MODEM_COMMAND_BUFFER_H
