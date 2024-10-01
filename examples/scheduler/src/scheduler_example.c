#include <stdio.h> // for sprintf
#include <string.h>

#include "pico/stdlib.h"

#include "scheduler_module.h"
#include "radio_module.h"

void send_message(char *message) {
	radio_send(message, strlen(message) + 1, 0x02);
}

void test_process(struct date_time_s *dt) {
	send_message("Hello");

	struct date_time_s add = { .minutes = 1 };
	date_time_add(dt, &add);
	schedule_process(dt, test_process);
}

int main() {
	scheduler_module_init();
	if (!radio_init()) goto IDLE_LOOP;
	radio_address_set(0x01);

	struct date_time_s sched = {0};
	struct date_time_s add = { .minutes = 1 };
	for (;;) {
		if (!scheduler_date_time_get(&sched)) {
			send_message("rtc failure\n");
			goto IDLE_LOOP;
		}

		date_time_add(&sched, &add);
		schedule_process(&sched, test_process);

		SCHEDULER_RETURN_T s_return = scheduler_run();

		switch (s_return) {
		case SCHEDULER_OK:
			send_message("scheduler ok\n");
			break;
		case RTC_FAILURE:
			send_message("rtc failure\n");
			goto IDLE_LOOP;
		}

		sleep_ms(1000);
	}

IDLE_LOOP:
	for (;;) sleep_ms(1000);

	return 0;
}
