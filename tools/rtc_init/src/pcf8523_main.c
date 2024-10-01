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
}

void gpio_callback_second(uint gpio, uint32_t event_mask) {
	pcf8523_second_int_flag_clear(I2C_NUM(I2C_INST));
	print_rtc_info(I2C_NUM(I2C_INST));
}

void gpio_callback_alarm(uint gpio, uint32_t event_mask) {
	printf("Alarm!\n");
	uint index = I2C_NUM(I2C_INST);
	pcf8523_alarm_int_flag_clear(index);
	pcf8523_alarm_int_disable(index);
	pcf8523_second_int_enable(index);
	gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback_second);
}

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
	//pcf8523_hour_mode_set(index, HOUR_MODE_12);
	//pcf8523_am_pm_set(index, PM);
	pcf8523_seconds_set(index, 55);
	pcf8523_minutes_set(index, 52);
	pcf8523_hours_set(index, 4);
	pcf8523_days_set(index, 2);
	pcf8523_months_set(index, 9);
	pcf8523_weekdays_set(index, MONDAY);
	pcf8523_years_set(index, 24);

	// Alarm
	pcf8523_minute_alarm_set(index, 53);
	pcf8523_hour_alarm_set(index, 4);
	pcf8523_day_alarm_set(index, 2);
	pcf8523_weekday_alarm_set(index, MONDAY);

	pcf8523_minute_alarm_enable(index);
	pcf8523_hour_alarm_enable(index);
	pcf8523_day_alarm_enable(index);
	pcf8523_weekday_alarm_enable(index);

	//pcf8523_time_circuit_stop(index);
	pcf8523_clockout_freq_set(index, CLOCKOUT_DISABLED);

	// IRQ
	gpio_pull_up(2);
	pcf8523_alarm_int_enable(index);
	gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback_alarm);
	//pcf8523_second_int_enable(index);
	//gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback_second);

	for(int i = 0;; i++) {
		//print_rtc_info(index);
		sleep_ms(1000);
	}
}

