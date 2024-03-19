#include "sensors.h"

size_t sensor_reading_to_bytes(
		sensor_reading_t *reading, 
		size_t dst_len, 
		uint8_t dst[dst_len]
) 
{
	switch(reading->type) {
	case TEROS_11:
		return teros_11_reading_to_bytes(
				(teros_11_reading_t *)reading,
				dst_len,
				dst
		);
	}
}

size_t teros_11_reading_to_bytes(
		teros_11_reading_t *reading,
		size_t dst_len, 
		uint8_t dst[dst_len]
) 
{
	
}
