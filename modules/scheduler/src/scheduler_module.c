#include <stdio.h>
#include <stddef.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"

#include "scheduler_module.h"
#include "pcf8523_rp2040.h"
#include "hibernate.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)
#define PIN_IRQ  (2)

struct schedule_entry {
	uint process_id;
	struct date_time_s sched_time;
	void (*process)(struct date_time_s *dt);
	struct schedule_entry *next;
};

static struct schedule_entry se_alloc_buffer[PROCESS_QUEUE_MAX] = {0};
static bool se_alloc_register[PROCESS_QUEUE_MAX] = {0};

struct schedule_list {
	uint size;
	struct schedule_entry *head;
};

static struct schedule_list process_queue = {
	.size = 0,
	.head = NULL
};

static void process_stub(struct date_time_s *dt) {};

void scheduler_module_init(void) {
	for (int i = 0; i < PROCESS_QUEUE_MAX; i++) {
		// Init all allocated schedule entries
		se_alloc_buffer[i].process_id = i;
		se_alloc_buffer[i].process = &process_stub;
		se_alloc_buffer[i].next = NULL;

		// Mark all entries as available
		se_alloc_register[i] = true;
	}

	// I2C to talk with RTC
	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);
	gpio_pull_up(PIN_IRQ);
}

static void process_queue_add(struct schedule_entry *entry) {
	process_queue.size++;

	// If queue is empty.
	if (process_queue.head == NULL) {
		process_queue.head = entry;
		return;
	}

	// If this should be new head (next process).
	if (date_time_cmp(&entry->sched_time, &process_queue.head->sched_time) < 0) {
		entry->next = process_queue.head;
		process_queue.head = entry;
		return;
	}

	// Find place in queue
	struct schedule_entry *ep = process_queue.head;

	for (;ep->next != NULL; ep = ep->next) {
		if (date_time_cmp(&entry->sched_time, &ep->next->sched_time) < 0) {
			entry->next = ep->next;
			ep->next = entry;
			return;
		}
	}
	// Hit end of queue
	ep->next = entry;
}

static struct schedule_entry *get_next_available_entry(void) {
	struct schedule_entry *entry = NULL;	

	for (int i = 0; i < PROCESS_QUEUE_MAX; i++) {
		if (se_alloc_register[i] == true) {
			se_alloc_register[i] = false;
			entry = &se_alloc_buffer[i];
			break;
		}
	}

	return entry;
}

static void reset_entry(struct schedule_entry *entry) {
		entry->process = &process_stub;
		entry->next = NULL;
		se_alloc_register[entry->process_id] = true;
}

bool scheduler_date_time_get(struct date_time_s *dst) {
	bool success = false;

	uint index = I2C_NUM(I2C_INST);
	// Check if rtc is responding and if clock time can be trusted
	// otherwise return false.
	bool is_set = false;
    if (!pcf8523_ci_warning_flag_is_set(index, &is_set))
		goto RETURN;
	if (is_set) goto RETURN;

	pcf8523_minutes_get(index, &dst->minutes);	
	pcf8523_hours_get(index, &dst->hours);	

	MONTH_T months;
	pcf8523_months_get(index, &months);
	dst->months = months;

	pcf8523_days_get(index, &dst->days);
	pcf8523_years_get(index, &dst->years);

	dst->century = false;

	success = true;
RETURN:
	return success;
}

bool schedule_process(
		struct date_time_s *sched_time, 
		void (*process)(struct date_time_s *)
)
{
	struct schedule_entry *new_entry = get_next_available_entry();
	// process queue full (see: PROCESS_QUEUE_MAX)
	if (new_entry == NULL) return false;

	// Copy passed time
	new_entry->sched_time = *sched_time;
	new_entry->process = process;

	process_queue_add(new_entry);

	return true;
}

static bool next_process_ready(struct date_time_s *now) {
	if (process_queue.head == NULL) return false;
	// Is the next scheduled process time <= now
	return date_time_cmp(&process_queue.head->sched_time, now) <= 0;
}

static void execute_next_process(void) {
	process_queue.size--;
	// Pop next process from the queue
	struct schedule_entry *ep = process_queue.head;
	process_queue.head = process_queue.head->next;

	printf("Executing process: %u\n", ep->process_id);
	// Execute process
	ep->process(&ep->sched_time);


	// Reset schedule_entry
	reset_entry(ep);
}

static void hibernate_until_next(void) {
	if (process_queue.head == NULL) {
		sleep_ms(100);
		return;
	}

	uint index = I2C_NUM(I2C_INST);

	struct date_time_s *dt = &process_queue.head->sched_time;

	pcf8523_minute_alarm_set(index, dt->minutes);
	pcf8523_hour_alarm_set(index, dt->hours);
	pcf8523_day_alarm_set(index, dt->days);

	pcf8523_minute_alarm_enable(index);
	pcf8523_hour_alarm_enable(index);
	pcf8523_day_alarm_enable(index);
	// enable alarm and clear flag
	pcf8523_alarm_int_enable(index);
	pcf8523_alarm_int_flag_clear(index);
	// Save old clocks
	uint clock0_orig = clocks_hw->sleep_en0;
	uint clock1_orig = clocks_hw->sleep_en1;

	// Hibernate until next
	hibernate_run_from_dormant_source(DORMANT_SOURCE_XOSC);
	hibernate_goto_dormant_until_pin(PIN_IRQ, true, false);
	hibernate_recover_clocks(clock0_orig, clock1_orig);
	// disable alarm
	pcf8523_alarm_int_disable(index);
}

SCHEDULER_RETURN_T scheduler_run(void) {
	SCHEDULER_RETURN_T rval = SCHEDULER_OK;

	struct date_time_s now;
	for (;;) {
		if (!scheduler_date_time_get(&now)) {
			rval = RTC_FAILURE;
			break;
		}

		while (next_process_ready(&now))
			execute_next_process();

		// If there are no process entries, the scheduler can stop.
		// Returns SCHEDULER_OK
		if (process_queue.head == NULL) break;

		hibernate_until_next();
	}

	return rval;
}
