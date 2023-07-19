#include <stdlib.h>
#include "command_buffer.h"

struct _command_buffer {
	// + 2 because the AT prefix is not counted in this limit
	uint8_t buffer[COMMAND_BUFFER_MAX + 2];
	uint8_t *index;
	bool at_prefix;
};

CommandBuffer *command_buffer_create() {
	CommandBuffer *cb = malloc((sizeof *cb));
	if (!cb) return NULL;

	cb->index = cb->buffer;
	cb->at_prefix = false;

	return cb;
}

void command_buffer_destroy(CommandBuffer *cb) {
	free(cb);
}

uint32_t command_buffer_write(CommandBuffer *cb, uint8_t *src, uint32_t src_len) {
	uint32_t written;
	for (written = 0; written < src_len ; written++) {
		if (command_buffer_full(cb)) break;

		*cb->index++ = src[written];
	}

	return written;
}

uint32_t command_buffer_length(CommandBuffer *cb) { 
	return cb->index - cb->buffer;
}

bool command_buffer_full(CommandBuffer *cb) {
	uint32_t buf_len = command_buffer_length(cb);

	if (cb->at_prefix) 
		return command_buffer_length(cb) >= COMMAND_BUFFER_MAX + 2;

	return command_buffer_length(cb) >= COMMAND_BUFFER_MAX;
}

bool command_buffer_empty(CommandBuffer *cb) {
	return command_buffer_length(cb) == 0;
}

uint8_t *command_buffer_get(CommandBuffer *cb) {
	return cb->buffer;
}

void command_buffer_clear(CommandBuffer *cb) {
	cb->index = cb->buffer;	
	cb->at_prefix = false;
}

bool command_buffer_prefix_set(CommandBuffer *cb) {
	if (!command_buffer_empty(cb)) return false;

	command_buffer_write(cb, "AT", 2);

	cb->at_prefix = true;
}
