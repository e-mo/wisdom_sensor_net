#ifndef WISDOM_MODEM_COMMAND_BUFFER_H
#define WISDOM_MODEM_COMMAND_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define COMMAND_BUFFER_MAX 559 // per SIM7080_Series_AT_Command_Manual_V1.02.pdf
							   //
typedef struct _command_buffer {
	// + 2 because the AT prefix is not counted in this limit
	// + 1 for \r
	uint8_t buffer[COMMAND_BUFFER_MAX + 2];
	uint8_t *index;
	bool at_prefix;
} CommandBuffer;

CommandBuffer *cb_create();
void cb_destroy(CommandBuffer *cb);
uint32_t cb_write(CommandBuffer *cb, uint8_t *src, uint32_t src_len);

uint32_t cb_length(CommandBuffer *cb);
bool cb_full(CommandBuffer *cb);
bool cb_empty(CommandBuffer *cb);
uint8_t *cb_get_buffer(CommandBuffer *cb);
CommandBuffer *cb_reset(CommandBuffer *cb);
bool cb_at_prefix_set(CommandBuffer *cb);

#endif // WISDOM_MODEM_COMMAND_BUFFER_H
