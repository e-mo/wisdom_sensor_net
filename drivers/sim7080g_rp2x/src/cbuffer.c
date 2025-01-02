#include <stdlib.h>

#include "cbuffer.h"

void cbuffer_init(cbuffer_t *buffer, void *array, size_t array_size) {
	buffer->buff = array;

	buffer->end_p = &buffer->buff[array_size];
	buffer->base_p = buffer->buff;
	buffer->write_p = buffer->buff;
	buffer->write_index = 0;
}

size_t cbuffer_remaining(cbuffer_t *buffer) {
	return cbuffer_size(buffer) - cbuffer_length(buffer);
}

bool cbuffer_empty(cbuffer_t *buffer) {
	return cbuffer_remaining(buffer) == cbuffer_size(buffer);
}

size_t cbuffer_size(cbuffer_t *buffer) {
	if (buffer == NULL) return 0;
	return buffer->end_p - buffer->buff;
}

size_t cbuffer_length(cbuffer_t *buffer) {
	if (buffer == NULL) return 0;
	// Because we normalize on pop, the write index
	// is also the length in bytes
	return buffer->write_index;
}

int cbuffer_push(cbuffer_t *buffer, void *src, size_t size) {
	if (buffer == NULL || src == NULL) return -1;

	uint8_t *src_bytes = (uint8_t *)src;
	uint pushed = 0;
	for (int i = 0; i < size && cbuffer_remaining(buffer) > 0; i++) {
		*buffer->write_p = src_bytes[i];

		buffer->write_p++;
		buffer->write_index++;
		pushed++;

		// If we are at end of buffer, cycle to beginning
		if (buffer->write_p == buffer->end_p)
			buffer->write_p = buffer->buff;
	}


	return pushed;
}

int cbuffer_pop(cbuffer_t *buffer, void *dest, size_t num) {
	if (buffer == NULL || dest == NULL) return -1;

	uint8_t *dest_bytes = (uint8_t *)dest;
	int popped = 0;
	for (int i = 0; i < num && !cbuffer_empty(buffer); i++) {
		if (dest != NULL)
			dest_bytes[i] = *buffer->base_p;

		buffer->base_p++;
		popped++;
		buffer->write_index--;

		// If we are at end of buffer, cycle to beginning
		if (buffer->base_p == buffer->end_p)
			buffer->base_p = buffer->buff;
	}

	return popped;
}

