#ifndef WISDOM_SENSOR_INTERFACE_H
#define WISDOM_SENSOR_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#include "sensor_type.h"

typedef unsigned uint;
typedef uint16_t SENSOR_TYPE;

struct _sensor_generic {
	SENSOR_TYPE_T type;
	int (*pack_func)
		(struct _sensor_generic *, uint8_t *, uint);
	bool (*read_func)(struct _sensor_generic *);
	bool has_reading;
};

void sensor_data_init(
	struct _sensor_generic *sd,
	SENSOR_TYPE_T type,
	int (*pack_func)(struct _sensor_generic *, uint8_t *, uint),
	bool (*read_func)(struct _sensor_generic *)
);

struct _sensor_generic *si_dispatch_unpack(uint8_t *buffer);

#endif // WISDOM_SENSOR_INTERFACE_H
