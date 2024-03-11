#ifndef WISDOM_RADIO_INTERFACE_H
#define WISDOM_RADIO_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

#include "radio_error.h"

typedef unsigned uint;

// Runs init process on radio
// Can be called again to reset radio
bool radio_init(void);

bool radio_address_set(uint8_t address);

bool radio_send(void *payload, uint size, uint8_t address);
bool radio_recv(void *buffer, uint size, uint *received);

RADIO_ERROR_T radio_status(char dst[ERROR_STR_MAX]);

#endif // WISDOM_RADIO_INTERFACE_H
