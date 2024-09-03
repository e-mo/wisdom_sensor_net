#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "pcf8523_rp2040.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

#define ever ;;

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

const char *weekday_string_lookup[] = {
	[SUNDAY]	= "Sunday",
	[MONDAY]	= "Monday",
	[TUESDAY]	= "Tuesday",
	[WEDNESDAY] = "Wednesday",
	[THURSDAY]	= "Thursday",
	[FRIDAY]	= "Friday",
	[SATURDAY]	= "Saturday"
};

void main() {
	stdio_init_all();

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	uint index = I2C_NUM(I2C_INST);

	uint buf = 69;

	pcf8523_software_reset_initiate(index);

	pcf8523_ci_warning_flag_clear(index);
	pcf8523_hour_mode_set(index, HOUR_MODE_12);
	pcf8523_am_pm_set(index, PM);
	pcf8523_seconds_set(index, 4);
	pcf8523_minutes_set(index, 52);
	pcf8523_hours_set(index, 4);
	pcf8523_days_set(index, 2);
	pcf8523_months_set(index, 9);
	pcf8523_weekdays_set(index, MONDAY);
	pcf8523_years_set(index, 24);

	for(int i = 0;; i++) {

		uint seconds;
		pcf8523_seconds_get(index, &seconds);	
		uint minutes;
		pcf8523_minutes_get(index, &minutes);	
		uint hours;
		pcf8523_hours_get(index, &hours);	
		AM_PM_T am_pm;
		pcf8523_am_pm_get(index, &am_pm);

		// [HH:MM:SS] [AM|PM]
		printf("%02u:%02u:%02u", hours, minutes, seconds);
		printf(" %s\n", am_pm == AM ? "AM" : "PM");

		MONTH_T month;
		pcf8523_months_get(index, &month);

		WEEKDAY_T weekday;
		pcf8523_weekdays_get(index, &weekday);

		uint days;
		pcf8523_days_get(index, &days);

		uint years;
		pcf8523_years_get(index, &years);

		// [WEEKDAY] [MONTH] [DAY], [YEAR]
		printf("%s", weekday_string_lookup[weekday]);
		printf(" %s", month_string_lookup[month]);
		printf(" %u, %02u\n", days, years);




		//uint8_t reg;
		//if (!pcf8523_months_reg_get(index, &reg))
		//	printf("Unabled to read\n");

		//printf("reg: 0X%02X\n", reg);

		printf("\n");
		sleep_ms(1000);
	}
}

