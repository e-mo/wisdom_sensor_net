#ifndef WISDOM_GATEWAY_MODULE_H
#define WISDOM_GATEWAY_MODULE_H

#include <stdbool.h>

// Note: Any commands can fail if modem is busy
// TODO: add way to read return messages from gw_core
// ABANDON SHIP GO ONE CORE! I'll fix this later.

bool gateway_init(void);

// Issues modem startup command
int gateway_pump(void);

int gateway_state_get(void);

// For sending packed data buffers
int gateway_queue_push(void *data, uint32_t size);

// For receiving data from gateway
bool gateway_recv(void *data, uint size);


#endif // WISDOM_GATEWAY_MODULE_H
