#ifndef WISDOM_GATEWAY_QUEUE_H
#define WISDOM_GATEWAY_QUEUE_H

#include "pico/util/queue.h"

#define GW_QUEUE_SIZE 100
#define GW_QUEUE_TYPE uint32_t
#define GW_QUEUE_WIDTH (sizeof (GW_QUEUE_TYPE))

void gw_queue_init(void);

int gw_queue_gw_peek(uint32_t *buffer);
int gw_queue_gw_send(uint8_t *buffer, uint16_t size);
int gw_queue_gw_recv(uint8_t *buffer, uint16_t size);

int gw_queue_main_send(uint8_t *buffer, uint16_t size);
int gw_queue_main_recv(uint8_t *buffer, uint16_t size);

#endif // WISDOM_GATEWAY_QUEUE_H
