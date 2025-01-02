#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "pcf8523_rp2040.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

#define ever ;;


const char *weekday_string_lookup[] = {
	[SUNDAY]	= "Sunday",
	[MONDAY]	= "Monday",
	[TUESDAY]	= "Tuesday",
	[WEDNESDAY] = "Wednesday",
	[THURSDAY]	= "Thursday",
	[FRIDAY]	= "Friday",
	[SATURDAY]	= "Saturday"
};

void print_rtc_info(uint index) {
	uint8_t seconds;
	pcf8523_seconds_get(index, &seconds);	
	uint8_t minutes;
	pcf8523_minutes_get(index, &minutes);	
	uint8_t hours;
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
	uint8_t days;
	pcf8523_days_get(index, &days);
	uint8_t years;
	pcf8523_years_get(index, &years);

	pcf8523_second_int_enable(index);

	// [WEEKDAY] [MONTH] [DAY], [YEAR]
	printf("%s", weekday_string_lookup[weekday]);
	printf(" %s", month_string_lookup[month]);
	printf(" %u, %02u\n", days, years);


	printf("\n");
}

void alarm(uint pin, uint32_t event) {
	uint index = I2C_NUM(I2C_INST);
	print_rtc_info(index);
	pcf8523_second_int_flag_clear(index);
}

void main() {
	stdio_init_all();

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);
	gpio_pull_up(2);

	uint index = I2C_NUM(I2C_INST);

	//gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_FALL, true, &alarm);
	//pcf8523_second_int_flag_clear(index);

	for(int i = 0;; i++) {
		print_rtc_info(index);

		sleep_ms(1000);
	}

}

