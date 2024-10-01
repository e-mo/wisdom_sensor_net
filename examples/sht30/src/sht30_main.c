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

#include "sht30_rp2040.h"

#define PIN_SCL (5)
#define PIN_SDA (4)

int main() {
    stdio_init_all(); // To be able to use printf

	i2c_init(i2c0, 100 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);

	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	struct sht30_rp2040_reading_s reading = {0};	

	for(;;) {
		// Wait for USB serial connection
		while (!tud_cdc_connected()) { sleep_ms(100); };
		
		sht30_rp2040_read(0, &reading);

		printf("Temp: %f\nHumidity: %f\n",
				reading.temperature, reading.humidity);

		sleep_ms(1000);
	}

ERROR_LOOP:
	for(;;) {
		printf("Error\n");
		sleep_ms(1000);
	}
}
