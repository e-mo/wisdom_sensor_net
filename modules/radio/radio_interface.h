#ifndef WISDOM_RADIO_INTERFACE_H
#define WISDOM_RADIO_INTERFACE_H

// Runs init process on radio
// Can be called again to reset radio
bool radio_init(void);

bool radio_address_set(uint8_t address);

bool radio_send(void *payload, uint size, uint8_t address);
bool radio_recv(void *buffer, uint size, uint *received);

#endif // WISDOM_RADIO_INTERFACE_H
