// radio_rx.c

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tusb.h"

#include "wisdom_sensors.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

int main() {
	stdio_init_all();

	sht30_wsi_t sht30 = {0};
	sht30_wsi_init(&sht30, 0);

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	sht30_wsi_t *sensor;
	
	int i = 1;
	uint8_t buf[100] = {0};
	for (int i = 1;; i++) {
		sensor_read((sensor_t *)&sht30);
		printf("before t:%f\n", sht30.reading.temperature);
		printf("before h:%f\n", sht30.reading.temperature);
		sensor_pack((sensor_t *)&sht30, buf, 100);
		sensor = (sht30_wsi_t *)sensor_unpack(buf);
		printf("after t:%f\n", sensor->reading.temperature);
		printf("after h:%f\n", sensor->reading.temperature);
		free(sensor);
		sleep_ms(1000);
	}
    
    return 0;
}
