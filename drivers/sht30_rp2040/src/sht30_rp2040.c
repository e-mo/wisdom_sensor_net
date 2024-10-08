#include <stdlib.h>

#include "hardware/i2c.h"

#include "sht30_rp2040.h"
#include "sht30_constants.h"

#define SHT30_I2C_ADDRESS (0x44)

const uint8_t sht30_command_lookup[SHT30_COMMANDS_MAX][2] = {
	[SHT30_SSDA_CS_HIGH] = {0x2C, 0x06},
	[SHT30_SSDA_CS_MED] = {0x2C, 0x0D},
	[SHT30_SSDA_CS_LOW] = {0x2C, 0x10},
	[SHT30_SSDA_HIGH] = {0x24, 0x00},
	[SHT30_SSDA_MED] = {0x24, 0x0B},
	[SHT30_SSDA_LOW] = {0x24, 0x16},
};

bool sht30_rp2040_read(uint i2c_index, struct sht30_reading_s *reading) {

	uint8_t read_buffer[SHT30_READING_SIZE] = {0x00};

	int rval = i2c_write_blocking(I2C_INSTANCE(i2c_index), SHT30_I2C_ADDRESS, 
			sht30_command_lookup[SHT30_SSDA_CS_HIGH], SHT30_COMMAND_SIZE, false);
	if (rval != SHT30_COMMAND_SIZE) return false;
	
	rval = i2c_read_blocking(I2C_INSTANCE(i2c_index), SHT30_I2C_ADDRESS, read_buffer, 
			SHT30_READING_SIZE, false);
	if (rval != SHT30_READING_SIZE) return false;

	reading->temperature = 
		-45 + (175 * (read_buffer[0] << 8 | read_buffer[1])/ 65535.0);
	reading->humidity = 
		100 * (read_buffer[3] << 8 | read_buffer[4]) / 65535.0;
	
	return true;
}
