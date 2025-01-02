#include "atr_buffer.h"

void atr_buffer_init(struct atr_buffer *buffer, void *storage, size_t size) {
	cbuffer_init(buffer->buffer, storage, size);
}
