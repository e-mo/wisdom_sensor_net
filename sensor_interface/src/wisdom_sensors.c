#include "wisdom_sensors.h"
#include "sensor_interface.h"

int sensor_pack(sensor_t *sensor, uint8_t *buffer, uint buffer_len) {
	return sensor->pack_func(sensor, buffer, buffer_len);
}

sensor_t * sensor_unpack(uint8_t *buffer) {
	return si_dispatch_unpack(buffer);
}

bool sensor_read(sensor_t *sensor) {
	return sensor->read_func(sensor);
}
