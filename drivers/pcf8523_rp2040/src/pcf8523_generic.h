#ifndef PCF8523_INTERFACE_GENERIC_H
#define PCF8523_INTERFACE_GENERIC_H

#include <stdbool.h>
#include <stdint.h>

#include "pcf8523_definitions.h"

// All functions return true if i2c communication was successful
// and return false if i2c communication failed or was unreliable.

bool pcf8523_reg_get(uint i2c_inst, uint8_t reg, uint8_t *dst);
bool pcf8523_reg_set(uint i2c_inst, uint8_t reg, uint8_t src);

bool pcf8523_control_get_all(uint i2c_inst, uint8_t dst[3]);
bool pcf8523_control_set_all(uint i2c_inst, uint8_t src[3]);

bool pcf8523_control_reg_get(uint i2c_inst, uint reg, uint8_t *dst);
bool pcf8523_control_reg_set(uint i2c_inst, uint reg, uint8_t src);

// CONTROL_1

// sets bit 7 in control register 1 which sets the internal oscillator
// capacitor for quartz crystals with a corresponding load capacitance
//
// For correct use of functions use one of the enumerated constants:
typedef enum _CAP_SEL_MODE_E {
	CAP_SEL_7_PF    = 0, // 7pF load capacitance
	CAP_SEL_12_5_PF = 1  // 12.5pF load capacitance
} CAP_SEL_MODE_T;

bool pcf8523_cap_sel_get(uint i2c_inst, uint *cap_sel_mode);
bool pcf8523_cap_sel_set(uint i2c_inst, CAP_SEL_MODE_T cap_sel_mode);

bool pcf8523_time_circuit_is_running(uint i2c_inst, bool *is_running);

// Haha! Bit can be set. I'm just dumb.
bool pcf8523_time_circuit_start(uint i2c_inst);
bool pcf8523_time_circuit_stop(uint i2c_inst);

bool pcf8523_software_reset_initiate(uint i2c_inst);

typedef enum _HOUR_MODE_E {
	HOUR_MODE_24 = 0, // 0-23 hour mode
	HOUR_MODE_12 = 1  // 1-12 hour mode with AM/PM flag
} HOUR_MODE_T;
bool pcf8523_hour_mode_get(uint i2c_inst, HOUR_MODE_T *hour_mode);
bool pcf8523_hour_mode_set(uint i2c_inst, HOUR_MODE_T hour_mode);

bool pcf8523_second_int_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_second_int_enable(uint i2c_inst);
bool pcf8523_second_int_disable(uint i2c_inst);

bool pcf8523_alarm_int_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_alarm_int_enable(uint i2c_inst);
bool pcf8523_alarm_int_disable(uint i2c_inst);

bool pcf8523_correction_int_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_correction_int_enable(uint i2c_inst);
bool pcf8523_correction_int_disable(uint i2c_inst);

// CONTROL_2

bool pcf8523_wtimer_int_flag_is_set(uint i2c_inst, bool *flag);

bool pcf8523_wtimer_int_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_wtimer_int_enable(uint i2c_inst);
bool pcf8523_wtimer_int_disable(uint i2c_inst);

typedef enum _COUNTDOWN_TIME_E {
	COUNTDOWN_TIMER_A,
	COUNTDOWN_TIMER_B
} COUNTDOWN_TIMER_T;
bool pcf8523_ctimer_int_flag_is_set(uint i2c_inst, COUNTDOWN_TIMER_T timer, bool *flag);
bool pcf8523_ctimer_int_flag_clear(uint i2c_inst, COUNTDOWN_TIMER_T timer);

bool pcf8523_ctimer_int_is_enabled(uint i2c_inst, COUNTDOWN_TIMER_T timer, bool *is_enabled);
bool pcf8523_ctimer_int_enable(uint i2c_inst, COUNTDOWN_TIMER_T timer);
bool pcf8523_ctimer_int_disable(uint i2c_inst, COUNTDOWN_TIMER_T timer);

bool pcf8523_second_int_flag_is_set(uint i2c_inst, bool *flag);
bool pcf8523_second_int_flag_clear(uint i2c_inst);

bool pcf8523_alarm_int_flag_is_set(uint i2c_inst, bool *flag);
bool pcf8523_alarm_int_flag_clear(uint i2c_inst);

// CONTROL_3

typedef enum _PM_FUNCTION_E {
	PM_SO_STANDARD_BLD_ENABLED   = 0x0,
	PM_SO_DIRECT_BLD_ENABLED     = 0x1,
	PM_SO_DISABLED_BLD_ENABLED   = 0x2,

	PM_SO_STANDARD_BLD_DISABLED  = 0x4,
	PM_SO_DIRECT_BLD_DISABLED    = 0x5,
	PM_SO_DISABLED_BLD_DISABLED  = 0x7,
} PM_FUNCTION_T;
bool pcf8523_pm_function_get(uint i2c_inst, uint *pm_function);
bool pcf8523_pm_function_set(uint i2c_inst, PM_FUNCTION_T pm_function);

bool pcf8523_bso_flag_is_set(uint i2c_inst, bool *is_set);
bool pcf8523_bso_flag_clear(uint i2c_inst);

bool pcf8523_bl_flag_is_set(uint i2c_inst, bool *is_set);

bool pcf8523_bso_int_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_bso_int_enable(uint i2c_inst);
bool pcf8523_bso_int_disable(uint i2c_inst);

bool pcf8523_bl_int_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_bl_int_enable(uint i2c_inst);
bool pcf8523_bl_int_disable(uint i2c_inst);

// MISC CONTROL
// Technically a control bit, but resides in seconds register

bool pcf8523_ci_warning_flag_is_set(uint i2c_inst, bool *is_set);
bool pcf8523_ci_warning_flag_clear(uint i2c_inst);

// TIME DATE

struct pcf8523_time_s {
	uint hours;
	uint minutes;
	uint seconds;
};

struct pcf8523_date_s {
	uint day;
	uint month;
	uint year;
	uint weekday;
};

struct pcf8523_time_date_s {
	struct pcf8523_time_s time;
	struct pcf8523_date_s date;
};

bool pcf8523_time_date_reg_get_all(uint i2c_inst, uint8_t dst[7]);
bool pcf8523_time_date_reg_set_all(uint i2c_inst, uint8_t src[7]);

bool pcf8523_time_date_get_all(uint i2c_inst, struct pcf8523_time_date_s *td);
bool pcf8523_time_date_set_all(uint i2c_inst, struct pcf8523_time_date_s *td);

bool pcf8523_time_reg_get_all(uint i2c_inst, uint8_t dst[3]);
bool pcf8523_time_reg_set_all(uint i2c_inst, uint8_t src[3]);

bool pcf8523_time_get_all(uint i2c_inst, struct pcf8523_time_s *time);
bool pcf8523_time_set_all(uint i2c_inst, struct pcf8523_time_s *time);

bool pcf8523_date_reg_get_all(uint i2c_inst, uint8_t dst[4]);
bool pcf8523_date_reg_set_all(uint i2c_inst, uint8_t src[4]);

bool pcf8523_date_get_all(uint i2c_inst, struct pcf8523_date_s *date);
bool pcf8523_date_set_all(uint i2c_inst, struct pcf8523_date_s *date);

bool pcf8523_seconds_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_seconds_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_seconds_get(uint i2c_inst, uint *seconds);
bool pcf8523_seconds_set(uint i2c_inst, uint seconds);

bool pcf8523_minutes_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_minutes_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_minutes_get(uint i2c_inst, uint *minutes);
bool pcf8523_minutes_set(uint i2c_inst, uint minutes);

bool pcf8523_hours_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_hours_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_hours_get(uint i2c_inst, uint *hours);
bool pcf8523_hours_set(uint i2c_inst, uint hours);

typedef enum _AM_PM_E {
	AM = 0,
	PM = 1
} AM_PM_T;
bool pcf8523_am_pm_get(uint i2c_inst, AM_PM_T *am_pm);
bool pcf8523_am_pm_set(uint i2c_inst, AM_PM_T am_pm);

bool pcf8523_days_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_days_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_days_get(uint i2c_inst, uint *day);
bool pcf8523_days_set(uint i2c_inst, uint day);

typedef enum _WEEKDAY_E {
	SUNDAY = 0,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
} WEEKDAY_T;
bool pcf8523_weekdays_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_weekdays_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_weekdays_get(uint i2c_inst, WEEKDAY_T *weekday);
bool pcf8523_weekdays_set(uint i2c_inst, WEEKDAY_T weekday);

typedef enum _MONTH_E {
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
} MONTH_T;
bool pcf8523_months_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_months_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_months_get(uint i2c_inst, MONTH_T *month);
bool pcf8523_months_set(uint i2c_inst, MONTH_T month);

bool pcf8523_years_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_years_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_years_get(uint i2c_inst, uint *years);
bool pcf8523_years_set(uint i2c_inst, uint years);

// Minute alarm
bool pcf8523_minute_alarm_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_minute_alarm_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_minute_alarm_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_minute_alarm_enable(uint i2c_inst);
bool pcf8523_minute_alarm_disable(uint i2c_inst);
bool pcf8523_minute_alarm_get(uint i2c_inst, uint *minute);
bool pcf8523_minute_alarm_set(uint i2c_inst, uint minute);

// Hour alarm
bool pcf8523_hour_alarm_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_hour_alarm_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_hour_alarm_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_hour_alarm_enable(uint i2c_inst);
bool pcf8523_hour_alarm_disable(uint i2c_inst);
bool pcf8523_hour_alarm_get(uint i2c_inst, uint *hour);
bool pcf8523_hour_alarm_set(uint i2c_inst, uint hour);

// Day alarm
bool pcf8523_day_alarm_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_day_alarm_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_day_alarm_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_day_alarm_enable(uint i2c_inst);
bool pcf8523_day_alarm_disable(uint i2c_inst);
bool pcf8523_day_alarm_get(uint i2c_inst, uint *day);
bool pcf8523_day_alarm_set(uint i2c_inst, uint day);

// Weekday alarm
bool pcf8523_weekday_alarm_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_weekday_alarm_reg_set(uint i2c_inst, uint8_t reg);

bool pcf8523_weekday_alarm_is_enabled(uint i2c_inst, bool *is_enabled);
bool pcf8523_weekday_alarm_enable(uint i2c_inst);
bool pcf8523_weekday_alarm_disable(uint i2c_inst);
bool pcf8523_weekday_alarm_get(uint i2c_inst, WEEKDAY_T *weekday);
bool pcf8523_weekday_alarm_set(uint i2c_inst, WEEKDAY_T weekday);

// Offset
typedef enum _OFFSET_MODE_E {
	OFFSET_EVERY_2_HOURS = 0,
	OFFSET_EVERY_MINUTE  = 1
} OFFSET_MODE_T;
bool pcf8523_offset_mode_get(uint i2c_inst, uint *mode);
bool pcf8523_offset_mode_set(uint i2c_inst, OFFSET_MODE_T mode);
// [-64:63]
bool pcf8523_offset_set(uint i2c_inst, int offset);

// Timer register

bool pcf8523_tmr_clkout_ctrl_reg_get(uint i2c_inst, uint8_t *reg);
bool pcf8523_tmr_clkout_ctrl_reg_set(uint i2c_inst, uint8_t reg);

typedef enum _CLOCKOUT_FREQ_E {
	CLOCKOUT_FREQ_32_768_HZ = 0x00,
	CLOCKOUT_FREQ_16_384_HZ = 0x01,
	CLOCKOUT_FREQ_8_192_HZ  = 0x02,
	CLOCKOUT_FREQ_4_096_HZ  = 0x03,
	CLOCKOUT_FREQ_1_024_HZ  = 0x04,
	CLOCKOUT_FREQ_32_HZ		= 0x05,
	CLOCKOUT_FREQ_1_HZ		= 0x06,
	CLOCKOUT_DISABLED		= 0x07
} CLOCKOUT_FREQ_T;
bool pcf8523_clockout_freq_get(uint i2c_inst, CLOCKOUT_FREQ_T *clockout);
bool pcf8523_clockout_freq_set(uint i2c_inst, CLOCKOUT_FREQ_T clockout);

#endif // PCF8523_INTERFACE_GENERIC_H
