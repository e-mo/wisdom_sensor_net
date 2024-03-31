#include "gateway_queue.h"

static queue_t _queue_gw;
static queue_t _queue_main;

static bool _is_init = false;

void gw_queue_init(void) {
	queue_init(&_queue_gw, GW_QUEUE_WIDTH, GW_QUEUE_SIZE);
	queue_init(&_queue_main, GW_QUEUE_WIDTH, GW_QUEUE_SIZE);

	_is_init = true;
}

int gw_queue_gw_peek(uint32_t *buffer) {
	if (_is_init == false) return -4;
	if (buffer == NULL) return -1;

	return queue_try_peek(&_queue_gw, buffer);
}

int gw_queue_gw_send(uint8_t *buffer, uint16_t size) {
	if (_is_init == false) return -4;
	if (buffer == NULL) return -1;

	uint sent = 0;
	while (queue_try_add(&_queue_main, buffer)) {
		sent += GW_QUEUE_WIDTH;
		if (sent >= size) break;
		buffer += GW_QUEUE_WIDTH;
	}

	if (sent > size) sent = size;
	return sent;
}

int gw_queue_gw_recv(uint8_t *buffer, uint16_t size) {
	if (_is_init == false) return -4;
	if (buffer == NULL) return -1; 
	if (size < GW_QUEUE_WIDTH) return -2; // Buffer too small

	uint received = 0;
	while (queue_try_remove(&_queue_gw, buffer)) {
		received += GW_QUEUE_WIDTH;
		size -= received;
		// If there is not enough in the buffer to receive
		// without overflowing;
		if (size < sizeof (uint32_t)) break;
		buffer += GW_QUEUE_WIDTH;
	}

	return received;
}

int gw_queue_main_send(uint8_t *buffer, uint16_t size) {
	if (_is_init == false) return -4;
	if (buffer == NULL) return -1;

	uint sent = 0;
	while (queue_try_add(&_queue_gw, buffer)) {
		sent += GW_QUEUE_WIDTH;
		if (sent >= size) break;
		buffer += GW_QUEUE_WIDTH;
	}

	if (sent > size) sent = size;
	return sent;
}

int gw_queue_main_recv(uint8_t *buffer, uint16_t size) {
	if (_is_init == false) return -4;
	if (buffer == NULL) return -1; 
	if (size < GW_QUEUE_WIDTH) return -2; // Buffer too small

	uint received = 0;
	while (queue_try_remove(&_queue_main, buffer)) {
		received += GW_QUEUE_WIDTH;
		// If there is not enough in the buffer to receive
		// without overflowing;
		if (size < sizeof (uint32_t)) break;
		buffer += GW_QUEUE_WIDTH;
	}

	return received;
}
