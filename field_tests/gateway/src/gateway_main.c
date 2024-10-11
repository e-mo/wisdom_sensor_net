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
//#include "hardware/i2c.h"
//#include "tusb.h"

#include "gateway.h"
#include "radio.h"
#include "scheduler_module.h"
//#include "wisdom_sensors.h"

//#define I2C_INST (i2c0)
//#define PIN_SCL  (5)
//#define PIN_SDA  (4)

//sht30_wsi_t sht30 = {0};
struct date_time_s add = { .minutes = 1 };

void send_message(char *message) {
	radio_send(message, strlen(message) + 1, 0x02);
}

bool collect_data(void *buf, uint buf_len, uint *received) {

	return radio_recv(buf, buf_len, received);
}

void collect_and_send(struct date_time_s *dt) {
	send_message("farting...");
	static uint8_t data[1024] = {0};
	uint received = 0;

	if (!collect_data(data, 1024, &received)) {
		send_message("SHIT EVERYWHERE!");
		return;
	}

	send_message("Received some stuff...");

	gateway_queue_push(data, received);

	//static uint16_t buf[100] = {1};
	//sensor_read((sensor_t *)&sht30);
	//buf[1] = sensor_pack((sensor_t *)&sht30, (uint8_t *)&buf[2], 100);
	//gateway_queue_push(&buf, buf[1] + 4);
	//int8_t now[5] = {0};
	//scheduler_date_time_get_packed(now);
	//gateway_queue_push(now, 5);

	int rval = gateway_pump();
	
	while (rval != MODEM_POWERED_DOWN) {
		switch (rval) {
		case MODEM_POWERED_DOWN:
			send_message("State: MODEM_POWERED_DOWN");
			break;
		case MODEM_STOPPED:
			send_message("State: MODEM_STOPPED");
			break;
		case MODEM_STARTED:
			send_message("State: MODEM_STARTED");
			break;
		case MODEM_CN_ACTIVE:
			send_message("State: MODEM_CN_ACTIVE");
			break;
		case MODEM_SERVER_CONNECTED:
			send_message("State: MODEM_SERVER_CONNECTED");
			break;
		}

		sleep_ms(500);
		rval = gateway_pump();
	}

	send_message("farted!");
	date_time_add(dt, &add);
	schedule_process(dt, collect_and_send);
}

int main() {
	// Wait for USB serial connection
	//while (!tud_cdc_connected()) { sleep_ms(100); };
	
	// Sechduler and radio init
	scheduler_module_init();
	if (!radio_init()) goto IDLE_LOOP;
	// Gateway address 0
	radio_address_set(0x00);

	// Gate init
	if (!gateway_init()) {
		send_message("gateway_init fail");
		goto IDLE_LOOP;
	}

	//i2c_init(I2C_INST, 500 * 1000);
	//gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	//gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	//gpio_pull_up(PIN_SCL);
	//gpio_pull_up(PIN_SDA);

	// Sensor init
	//sht30_wsi_init(&sht30, 0);

	// date_time objects
	struct date_time_s sched = {0};

	send_message("FART!");

	for (;;) {
		if (!scheduler_date_time_get(&sched)) {
			send_message("rtc failure");
			goto IDLE_LOOP;
		}

		date_time_add(&sched, &add);
		schedule_process(&sched, collect_and_send);

		SCHEDULER_RETURN_T s_return = scheduler_run();

		switch (s_return) {
		case SCHEDULER_OK:
			send_message("scheduler ok\n");
			break;
		case RTC_FAILURE:
			send_message("rtc failure\n");
			goto IDLE_LOOP;
		}

		//sleep_ms(100);
	}
IDLE_LOOP:
	for (;;) sleep_ms(1000);
    
    return 0;
}
