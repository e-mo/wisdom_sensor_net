#ifndef ATR_BUFFER_H
#define ATR_BUFFER_H

#include "cbuffer.h"

typedef struct atr_buffer {
	cbuffer_t storage;
	uint buffered;
} atr_buffer_t;


#endif // ATR_BUFFER_H
