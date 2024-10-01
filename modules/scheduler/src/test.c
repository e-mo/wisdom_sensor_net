#include <stdio.h>
#include "date_time.h"
#include "scheduler_module.h"

void process(struct date_time_s *dt) {
	printf("Hello!\n");
	schedule_process(dt, &process);
}

int main() {
	
	struct date_time_s sched = {
		.seconds = 1,
		.months = 1
	};
	scheduler_module_init();
	schedule_process(&sched, &process);

	scheduler_run();

	return 0;
}
