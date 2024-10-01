#ifndef SHT30_RP2040_WISDOM_H
#define SHT30_RP2040_WISDOM_H

#include <stdbool.h>

#define SHT30_READING_SIZE 6
#define SHT30_COMMAND_SIZE 2

typedef unsigned uint;

struct sht30_rp2040_reading_s {
	float temperature;
	float humidity;
};

bool sht30_rp2040_read(uint i2c_index, struct sht30_rp2040_reading_s *reading);

#endif //SHT30_RP2040_WISDOM_H
