#ifndef SCHEDULER_MODULE_H
#define SCHEDULER_MODULE_H

#include <stdbool.h>

#include "date_time.h"

#define PROCESS_QUEUE_MAX (10)

typedef unsigned uint;

// Initialized schedule list and allocation buffer
void scheduler_module_init(void);

typedef enum scheduler_return_e {
	SCHEDULER_OK,
	RTC_FAILURE
} SCHEDULER_RETURN_T;
SCHEDULER_RETURN_T scheduler_run(void);

bool scheduler_date_time_get(struct date_time_s *dst);

bool schedule_process(
		struct date_time_s *sched_time, 
		void (*process)(struct date_time_s *)
);

#endif // SCHEDULER_MODULE_H
