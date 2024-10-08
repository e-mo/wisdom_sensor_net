// template_main.c

//	Copyright (C) 2024 
//	Evan Morse
//	Amelia Vlahogiannis
//	Noelle Steil
//	Jordan Allen
//	Sam Cowan
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

#include "gateway.h"
#include "wisdom_sensors.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

int main() {
    stdio_init_all(); // To be able to use printf
	
	// Wait for USB serial connection
	while (!tud_cdc_connected()) { sleep_ms(100); };

	gateway_init();

	sht30_wsi_t sht30 = {0};
	sht30_wsi_init(&sht30, 0);

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	uint16_t buf[100] = {1};
	for (;;) {
		sensor_read((sensor_t *)&sht30);
		buf[1] = sensor_pack((sensor_t *)&sht30, buf, 100);
		memcpy(&buf[2], buf, buf[1]);
		gateway_send(&buf, buf[1]);
		sleep_ms(60000);
	}
    
    return 0;
}
