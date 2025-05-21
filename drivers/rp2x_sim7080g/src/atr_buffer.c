#include "atr_buffer.h"

static inline 
void atr_buffer_init(atr_buffer_t *buffer, void *array, size_t array_size) {
	cbuffer_init(buffer, array, array_size);
}

static inline
int atr_buffer_push(atr_buffer_t *buffer, void *src, size_t size) {
	return cbuffer_push(cbuffer_t *buffer, void *src, size_t size);
}
