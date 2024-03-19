#ifndef WISDOM_SENSORS_H
#define WISDOM_SENSOR_H

typedef enum sensor_code {
	TEROS_11
} sensor_t;

typedef struct sensor_reading {
	sensor_t type;
	uint64_t timestamp;
} sensor_reading_t;

typedef struct teros_11_reading {
	sensor_t type;
	uint64_t timestamp;
	float temperature;
	float vwc;
} teros_11_reading_t;

size_t sensor_reading_to_bytes(
		sensor_reading_t *reading, 
		size_t dst_len, 
		uint8_t dst[dst_len]
);

size_t teros_11_reading_to_bytes(
		teros_11_reading_t *reading,
		size_t dst_len, 
		uint8_t dst[dst_len]
);

#endif // WISDOM_SENSOR_H
