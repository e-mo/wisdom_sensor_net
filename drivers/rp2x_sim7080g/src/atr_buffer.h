#ifndef ATR_BUFFER_H
#define ATR_BUFFER_H

#include "cbuffer.h"
typedef cbuffer_t atr_buffer_t;

static inline
void atr_buffer_init(atr_buffer_t *buffer, void *array, size_t array_size);

static inline
int atr_buffer_push(atr_buffer_t *buffer, void *src, size_t size);


#endif // ATR_BUFFER_H
