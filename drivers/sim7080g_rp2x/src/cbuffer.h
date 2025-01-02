#ifndef CIRCLE_BUFFER_BROG_H
#define CIRCLE_BUFFER_BROG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef unsigned uint;

typedef struct _circle_buffer {
	uint8_t *buff; // Pointer to beginning of buffer array
	uint8_t *end_p;  // Pointer to end of buffer array
	
	// The current base
	uint8_t *base_p;

	// Current write head
	uint8_t *write_p;
	size_t write_index;
} cbuffer_t;

void cbuffer_init(cbuffer_t *buffer, void *array, size_t array_size);

// Returns remaining buffer space in bytes
size_t cbuffer_remaining(cbuffer_t *buffer);
// Returns true if buffer is empty
// else false
bool cbuffer_empty(cbuffer_t *buffer);

// Returns fixed size of buffer in bytes
// Returns current length of buffer in bytes
size_t cbuffer_size(cbuffer_t *buffer);
size_t cbuffer_length(cbuffer_t *buffer);

// Pushes up to [size] bytes from [src] into [buffer]
// Returns number of bytes pushed
// -1 if [buffer] or [src] is NULL
int cbuffer_push(cbuffer_t *buffer, void *src, size_t size);

// Pop up to [num] bytes from [buffer] into [dest]
// Returns number of bytes popped
// -1 if [buffer] or [dest] is NULL
int cbuffer_pop(cbuffer_t *buffer, void *dest, size_t num);

#endif // CIRCLE_BUFFER_BROG_H
