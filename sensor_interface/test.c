#include <stdio.h>

#include "src/wisdom_sensors.h"

int main() {
	
	test_sensor_t *ts = test_sensor_create();
	sensor_t *sensor = (sensor_t *)ts;
	
	// Get reading from sensor
	if (!sensor_read(sensor)) return 1;

	printf("Reading before packing: %u\n", ts->reading);
	
	// Pack sensor data into buffer
	uint8_t buffer[100] = {0};
	int packed = sensor_pack(sensor, buffer, 100);
	if (packed <= 0) return 2;

	printf("Bytes packed: %i\n", packed);

	// Free sensor just because
	test_sensor_destroy(ts);
	ts = NULL;
	sensor = NULL;

	// Unpack sensor and prove data hasn't changed
	sensor = sensor_unpack(buffer);
	ts = (test_sensor_t *)sensor;

	printf("Reading after unpacking: %u\n", ts->reading);

	return 0;
}
