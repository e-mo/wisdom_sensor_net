#include <stdlib.h>
#include "command_buffer.h"

struct _command_buffer {
	// + 2 because the AT prefix is not counted in this limit
	// + 1 for \r
	uint8_t buffer[COMMAND_BUFFER_MAX + 2];
	uint8_t *index;
	bool at_prefix;
};

CommandBuffer *cb_create() {
	CommandBuffer *cb = malloc((sizeof *cb));
	if (!cb) return NULL;

	cb->index = cb->buffer;
	*cb->index = '\r';
	cb->at_prefix = false;

	return cb;
}

void cb_destroy(CommandBuffer *cb) {
	free(cb);
}

uint32_t cb_write(CommandBuffer *cb, uint8_t *src, uint32_t src_len) {
	uint32_t written;
	for (written = 0; written < src_len ; written++) {
		if (cb_full(cb)) break;

		*cb->index++ = src[written];
	}

	// Always leave a return character at end
	// which will be overwritten if the buffer is appended
	// to
	*cb->index = '\r';
	return written;
}

uint32_t cb_length(CommandBuffer *cb) { 
	// +1 for terminating '\r'
	return cb->index - cb->buffer + 1;
}

bool cb_full(CommandBuffer *cb) {
	uint32_t buf_len = cb_length(cb);

	if (cb->at_prefix) 
		return cb_length(cb) >= COMMAND_BUFFER_MAX + 2;

	return cb_length(cb) >= COMMAND_BUFFER_MAX;
}

bool cb_empty(CommandBuffer *cb) {
	return cb_length(cb) <= 1;
}

uint8_t *cb_get(CommandBuffer *cb) {
	return cb->buffer;
}

void cb_clear(CommandBuffer *cb) {
	cb->index = cb->buffer;	
	cb->at_prefix = false;
}

bool cb_prefix_set(CommandBuffer *cb) {
	if (!cb_empty(cb)) return false;

	cb_write(cb, "AT", 2);

	cb->at_prefix = true;
}
