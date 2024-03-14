#include <stddef.h>

#include "sensor_interface.h"
#include "sensor_unpack.h"

static SENSOR_TYPE si_packed_sensor_type(uint8_t *buffer);

static SENSOR_TYPE si_packed_sensor_type(uint8_t *buffer) {
	if (buffer == NULL) return -1;

	// Because the first thing in the buffer is the sensor type
	// we can simply cast a deref of the buffer
	return (SENSOR_TYPE) *buffer;
}

void sensor_data_init(
	struct _sensor_generic *sd,
	SENSOR_TYPE_T type,
	int (*pack_func)(struct _sensor_generic *, uint8_t *, uint),
	bool (*read_func)(struct _sensor_generic *)
)
{
	// TODO: add error handling to this function
	if (sd == NULL) return;

	sd->type = type;	
	sd->pack_func = pack_func;
	sd->read_func = read_func;
	sd->has_reading = false;
}

struct _sensor_generic * si_dispatch_unpack(uint8_t *buffer) {
	if (buffer == NULL) return NULL;

	SENSOR_TYPE type = si_packed_sensor_type(buffer);
	if (type < 0 || type > SENSOR_TYPE_MAX) return NULL;

	return (sensor_unpack_table[type])(buffer);
}
