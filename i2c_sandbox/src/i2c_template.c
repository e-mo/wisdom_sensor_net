// template_main.c

//	Copyright (C) 2024 
//	Evan Morse
//	Amelia Vlahogiannis
//	Noelle Steil
//	Jordan Allen
//	Sam Colwell
//	Rachel Cleminson

//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.

//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.

//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tusb.h"

// OPTIONAL INCLUDES
// Uncomment depending on libraries used

// RFM69 library
//#include "rfm69_pico.h" 

// SD card library
//#include "sd_config.h"

#define I2C_INST (i2c0)
#define I2C_ADDRESS (0x6F)
#define I2C_BAUD (100 * 1000)
#define PIN_SCL (5)
#define PIN_SDA (4)

int main() {
	// To be able to use printf
    stdio_init_all(); 

	// Wait for USB serial connection
	while (!tud_cdc_connected()) { sleep_ms(100); };

	uint baud = i2c_init(I2C_INST, I2C_BAUD);
	if (baud != I2C_BAUD) {
		printf("i2c_init failed to set desired baud\n");
		printf("desired: %u\n", I2C_BAUD);
	}
	printf("i2c baud set: %u\n", baud);

	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);

	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	uint8_t address = 0x00;
	uint8_t buf = 0xFF;

	// Enable st
	uint8_t enable_st[2] = {0x00, 0x01};

	//i2c_write_blocking(I2C_INST, I2C_ADDRESS, enable_st, 2, false);

	int i = 1;
	for(;;) {

		uint rval = i2c_read_blocking(I2C_INST, I2C_ADDRESS, &buf, 1, false);
		if (rval == PICO_ERROR_GENERIC) printf("unable to read\n");

		printf("buf: %u\n", buf);
		
		sleep_ms(1000);
	}
    
    return 0;
}
