#include <stdio.h>
#include "date_time.h"

// 12+1 because no 0 month
static uint days_in_month_lookup[12+1] = {
	[JANUARY]	= 31,
	[FEBRUARY]	= 28,
	[MARCH]		= 31,
	[APRIL]		= 30,
	[MAY]		= 31,
	[JUNE]		= 30,
	[JULY]      = 31,
	[AUGUST]    = 31,
	[SEPTEMBER] = 30,
	[OCTOBER]	= 31,
	[NOVEMBER]	= 30,
	[DECEMBER]	= 31
};

const char *month_string_lookup[] = {
	[JANUARY]	= "January", 
	[FEBRUARY]	= "February",
	[MARCH]		= "March",
	[APRIL]		= "April",
	[MAY]		= "May",
	[JUNE]		= "June",
	[JULY]		= "July",
	[AUGUST]	= "August",
	[SEPTEMBER] = "September",
	[OCTOBER]	= "October",
	[NOVEMBER]	= "November",
	[DECEMBER]	= "December"
};


// if return > 0, a after b
// if return < 0, a before b
// if return == 0, a == b
int date_time_cmp(struct date_time_s *dt_a, struct date_time_s *dt_b) {
	int diff = 0;

	diff = dt_a->century - dt_b->century;
	if (diff) goto RETURN;
	diff = dt_a->years - dt_b->years;
	if (diff) goto RETURN;
	diff = dt_a->months - dt_b->months;
	if (diff) goto RETURN;
	diff = dt_a->days - dt_b->days;
	if (diff) goto RETURN;
	diff = dt_a->hours - dt_b->hours;
	if (diff) goto RETURN;
	diff = dt_a->minutes - dt_b->minutes;

RETURN:
	return diff;
};

static uint days_in_month(struct date_time_s *dt) {
	// If it isn't a leap year, or it isn't february
	// just return expected number of days.
	if (dt->months != FEBRUARY || dt->years % 4 != 0) 
		return days_in_month_lookup[dt->months];

	// it is February AND a leap year
	return 29;
}

void date_time_add(struct date_time_s *dt, struct date_time_s *add) {

	dt->minutes += add->minutes;
	dt->hours += dt->minutes / 60;
	dt->minutes %= 60;

	dt->hours += add->hours;
	dt->days += dt->hours / 24;
	dt->hours %= 24;

	dt->days += add->days;
	while (dt->days > days_in_month(dt)) {
		dt->days -= days_in_month(dt);
		dt->months++;
		dt->years += (dt->months - 1) / 12;
		dt->months = ((dt->months - 1) % 12) + 1;
	}

	dt->months += add->months;
	dt->years += (dt->months - 1) / 12;
	dt->months = ((dt->months - 1) % 12) + 1;

	dt->years += add->years;
	dt->century = dt->years > 99;
	dt->years %= 100;
}

void date_time_print(struct date_time_s *dt) {
	printf("%02u:%02u", dt->hours, dt->minutes);
	printf("%s %02u, %02u\n", 
		month_string_lookup[dt->months], 
		dt->days, 
		dt->years
	);
	printf("Century: %s\n", dt->century ? "true" : "false");
}
