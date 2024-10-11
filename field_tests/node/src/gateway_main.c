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

//#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
//#include "tusb.h"

//#include "gateway.h"
#include "radio.h"
#include "scheduler_module.h"
#include "wisdom_sensors.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

sht30_wsi_t sht30 = {0};
struct date_time_s add = { .minutes = 1 };

void send_message(char *message) {
	radio_send(message, strlen(message) + 1, 0x02);
}

void send_reading(struct date_time_s *dt) {

	send_message("NODE SENDING");
	static uint16_t buf[1024] = {1};
	sensor_read((sensor_t *)&sht30);
	buf[1] = sensor_pack((sensor_t *)&sht30, (uint8_t *)&buf[2], 1024);
	uint8_t *bp = ((uint8_t *)buf) + buf[1] + 4;
	scheduler_date_time_get_packed(bp);
	if (!radio_send(buf, buf[1] + 4 + 5, 0x00))
		send_message("data send failed");

	date_time_add(dt, &add);
	schedule_process(dt, send_reading);
}

int main() {
	// Sechduler and radio init
	scheduler_module_init();
	if (!radio_init()) goto IDLE_LOOP;
	// Gateway address 0
	radio_address_set(0x01);

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	// Sensor init
	sht30_wsi_init(&sht30, 0);

	// date_time objects
	struct date_time_s sched = {0};

	send_message("FART NODE!");

	for (;;) {
		if (!scheduler_date_time_get(&sched)) {
			send_message("rtc failure");
			goto IDLE_LOOP;
		}

		date_time_add(&sched, &add);
		send_reading(&sched);

		SCHEDULER_RETURN_T s_return = scheduler_run();

		switch (s_return) {
		case SCHEDULER_OK:
			send_message("scheduler ok\n");
			break;
		case RTC_FAILURE:
			send_message("rtc failure\n");
			goto IDLE_LOOP;
		}

		sleep_ms(100);
	}
IDLE_LOOP:
	for (;;) sleep_ms(1000);
    
    return 0;
}
