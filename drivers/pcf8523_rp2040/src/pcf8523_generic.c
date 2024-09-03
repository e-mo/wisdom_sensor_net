#include <stdio.h>

#include "pcf8523_generic.h"
#include "pcf8523_i2c_generic.h"

bool pcf8523_reg_get(uint i2c_inst, uint8_t reg, uint8_t *dst) {
	bool success = false;

	if (!pcf8523_i2c_write(i2c_inst, &reg, 1))
		goto RETURN;	

	if (!pcf8523_i2c_read(i2c_inst, dst, 1))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_reg_set(uint i2c_inst, uint8_t reg, uint8_t src) {
	bool success = false;

	uint8_t buf[2] = {reg, src};
	if (!pcf8523_i2c_write(i2c_inst, buf, 2))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_control_get_all(uint i2c_inst, uint8_t dst[3]) {
	bool success = false;

	// Set address to first control register.
	uint8_t reg = PCF8523_REG_CONTROL_1;
	if (!pcf8523_i2c_write(i2c_inst, &reg, 1))
		goto RETURN;	

	// Read all three control registers.
	if (!pcf8523_i2c_read(i2c_inst, dst, 3))
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_control_set_all(uint i2c_inst, uint8_t src[3]) {
	bool success = false;

	// Create buffer with register address first.
	uint8_t buf[4] = {PCF8523_REG_CONTROL_1, src[0], src[1], src[2]};

	if (!pcf8523_i2c_write(i2c_inst, buf, 4))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}


bool pcf8523_control_reg_get(uint i2c_inst, uint reg, uint8_t *dst) {
	bool success = false;

	uint8_t address = 0xFF;
	switch (reg) {
	case 1:
		address = PCF8523_REG_CONTROL_1;
		break;
	case 2:
		address = PCF8523_REG_CONTROL_2;
		break;
	case 3:
		address = PCF8523_REG_CONTROL_3;
		break;
	default:
		goto RETURN;
	}

	if (!pcf8523_reg_get(i2c_inst, address, dst))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}


bool pcf8523_control_reg_set(uint i2c_inst, uint reg, uint8_t src) {
	bool success = false;

	uint8_t address = 0xFF;
	switch (reg) {
	case 1:
		address = PCF8523_REG_CONTROL_1;
		break;
	case 2:
		address = PCF8523_REG_CONTROL_2;
		break;
	case 3:
		address = PCF8523_REG_CONTROL_3;
		break;
	default:
		goto RETURN;
	}

	if (!pcf8523_reg_set(i2c_inst, address, src))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}


// typedef enum _CAP_SEL_MODE_E {
// 	CAP_SEL_7_PF    = 0, // 7pF load capacitance
// 	CAP_SEL_12_5_PF = 1  // 12.5pF load capacitance
// } CAP_SEL_MODE_T;
bool pcf8523_cap_sel_get(uint i2c_inst, uint *cap_sel_mode) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf = (buf & 0x80) >> 7;
	*cap_sel_mode = buf;

	success = true;
RETURN:
	return success;
}
 

bool pcf8523_cap_sel_set(uint i2c_inst, CAP_SEL_MODE_T cap_sel_mode) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf &= ~0x80;
	buf |= cap_sel_mode << 7;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_time_circuit_is_running(uint i2c_inst, bool *is_running) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	*is_running = !(buf & 0x20);

	success = true;
RETURN:
	return success;
}


//bool pcf8523_time_circuit_start(uint i2c_inst) {
//	bool success = false;
//
//	uint8_t buf;
//	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
//		goto RETURN;
//
//	// Set STOP bit
//	buf |= 0x20;
//
//	if (!pcf8523_control_reg_set(i2c_inst, 1, &buf))
//		goto RETURN;
//
//	success = true;
//RETURN:
//	return success;
//}
//bool pcf8523_time_circuit_stop(uint i2c_inst) {
//	bool success = false;
//
//	uint8_t buf;
//	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
//		goto RETURN;
//
//	// Clear STOP bit
//	buf &= ~0x20;
//
//	if (!pcf8523_control_reg_set(i2c_inst, 1, &buf))
//		goto RETURN;
//
//	success = true;
//RETURN:
//	return success;
//}


bool pcf8523_software_reset_initiate(uint i2c_inst) {
	bool success = false;

	uint8_t reset_command = 0x58; // See datasheet section 8.3
	if (!pcf8523_control_reg_set(i2c_inst, 1, reset_command))
		goto RETURN;

	success = true;
RETURN:
	return success;
};


//	typedef enum _HOUR_MODE_E {
//	 	HOUR_MODE_24 = 0, // 0-23 hour mode
//	 	HOUR_MODE_12 = 1  // 1-12 hour mode with AM/PM flag
//	} HOUR_MODE_T;
bool pcf8523_hour_mode_get(uint i2c_inst, HOUR_MODE_T *hour_mode) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf = (buf & 0x08) >> 3;
	*hour_mode = buf;

	success = true;
RETURN:
	return success;
}
 

bool pcf8523_hour_mode_set(uint i2c_inst, HOUR_MODE_T hour_mode) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf &= ~0x08;
	buf |= hour_mode << 3;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_second_int_is_enabled(uint i2c_inst, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	*is_enabled = !!(buf & 0x04);

	success = true;
RETURN:
	return success;
}


bool pcf8523_second_int_enable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf |= 0x04;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_second_int_disable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf &= ~0x04;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;


	success = true;
RETURN:
	return success;
}


bool pcf8523_alarm_int_is_enabled(uint i2c_inst, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	*is_enabled = !!(buf & 0x02);

	success = true;
RETURN:
	return success;
}


bool pcf8523_alarm_int_enable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf |= 0x02;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_alarm_int_disable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf &= ~0x02;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;


	success = true;
RETURN:
	return success;
}


bool pcf8523_correction_int_is_enabled(uint i2c_inst, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	*is_enabled = !!(buf & 0x01);

	success = true;
RETURN:
	return success;
}


bool pcf8523_correction_int_enable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf |= 0x01;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_correction_int_disable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 1, &buf))	
		goto RETURN;

	buf &= ~0x01;

	if (!pcf8523_control_reg_set(i2c_inst, 1, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_wtimer_int_flag_is_set(uint i2c_inst, bool *is_set) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	*is_set = !!(buf & 0x80);

	success = true;
RETURN:
	return success;
}


bool pcf8523_wtimer_int_is_enabled(uint i2c_inst, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;
	
	*is_enabled = !!(buf & 0x04);

	success = true;
RETURN:
	return success;
}


bool pcf8523_wtimer_int_enable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	buf |= 0x04;

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}


bool pcf8523_wtimer_int_disable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	buf &= ~0x04;

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

// typedef enum _COUNTDOWN_TIME_E {
//  COUNTDOWN_TIMER_A,
//  COUNTDOWN_TIMER_B
// } COUNTDOWN_TIMER_T;
bool pcf8523_ctimer_int_flag_is_set(uint i2c_inst, COUNTDOWN_TIMER_T timer, bool *flag) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	switch (timer) {
	case COUNTDOWN_TIMER_A:
		*flag = !!(buf & 0x40);
		break;
	case COUNTDOWN_TIMER_B:
		*flag = !!(buf & 0x20);
		break;
	default:
		goto RETURN;
	}

	success = true;
RETURN:
	return success;

}
bool pcf8523_ctimer_int_flag_clear(uint i2c_inst, COUNTDOWN_TIMER_T timer) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	switch (timer) {
	case COUNTDOWN_TIMER_A:
		buf &= ~0x40;
		break;
	case COUNTDOWN_TIMER_B:
		buf &= ~0x20;
		break;
	default:
		goto RETURN;
	}

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_ctimer_int_is_enabled(uint i2c_inst, COUNTDOWN_TIMER_T timer, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	switch (timer) {
	case COUNTDOWN_TIMER_A:
		*is_enabled = !!(buf & 0x02);
		break;
	case COUNTDOWN_TIMER_B:
		*is_enabled = !!(buf & 0x01);
		break;
	default:
		goto RETURN;
	}

	success = true;
RETURN:
	return success;

}

bool pcf8523_ctimer_int_enable(uint i2c_inst, COUNTDOWN_TIMER_T timer) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	switch (timer) {
	case COUNTDOWN_TIMER_A:
		buf |= 0x02;
		break;
	case COUNTDOWN_TIMER_B:
		buf |= 0x01;
		break;
	default:
		goto RETURN;
	}

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_ctimer_int_disable(uint i2c_inst, COUNTDOWN_TIMER_T timer) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	switch (timer) {
	case COUNTDOWN_TIMER_A:
		buf &= ~0x02;
		break;
	case COUNTDOWN_TIMER_B:
		buf &= ~0x01;
		break;
	default:
		goto RETURN;
	}

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_second_int_flag_is_set(uint i2c_inst, bool *is_set) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	*is_set = !!(buf & 0x10);

	success = true;
RETURN:
	return success;
}

bool pcf8523_second_int_flag_clear(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	buf &= ~0x10;

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_alarm_int_flag_is_set(uint i2c_inst, bool *is_set) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	*is_set = !!(buf & 0x08);

	success = true;
RETURN:
	return success;
}

bool pcf8523_alarm_int_flag_clear(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 2, &buf))	
		goto RETURN;

	buf &= ~0x08;

	if (!pcf8523_control_reg_set(i2c_inst, 2, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

// typedef enum _PM_FUNCTION_E {
// 	PM_SO_STANDARD_BLD_ENABLED   = 0x0,
// 	PM_SO_DIRECT_BLD_ENABLED     = 0x1,
// 	PM_SO_DISABLED_BLD_ENABLED   = 0x2,
// 
// 	PM_SO_STANDARD_BLD_DISABLED  = 0x4,
// 	PM_SO_DIRECT_BLD_DISABLED    = 0x5,
// 	PM_SO_DISABLED_BLD_DISABLED  = 0x7,
// } PM_FUNCTION_T;
bool pcf8523_pm_function_get(uint i2c_inst, uint *pm_function) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf &= 0xE0;
	*pm_function = buf >> 5;

	success = true;
RETURN:
	return success;
}

bool pcf8523_pm_function_set(uint i2c_inst, PM_FUNCTION_T pm_function) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf &= ~0xE0;
	buf |= pm_function << 5;

	if (!pcf8523_control_reg_set(i2c_inst, 3, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_bso_flag_is_set(uint i2c_inst, bool *is_set) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	*is_set = !!(buf & 0x08);

	success = true;
RETURN:
	return success;
}

bool pcf8523_bso_flag_clear(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf &= ~0x08;

	if (!pcf8523_control_reg_set(i2c_inst, 3, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_bl_flag_is_set(uint i2c_inst, bool *is_set) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	*is_set = !!(buf & 0x04);

	success = true;
RETURN:
	return success;
}

bool pcf8523_bso_int_is_enabled(uint i2c_inst, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	*is_enabled = !!(buf & 0x02);

	success = true;
RETURN:
	return success;
}

bool pcf8523_bso_int_enable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf |= 0x02;

	if (!pcf8523_control_reg_set(i2c_inst, 3, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_bso_int_disable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf &= ~0x02;

	if (!pcf8523_control_reg_set(i2c_inst, 3, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_bl_int_is_enabled(uint i2c_inst, bool *is_enabled) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	*is_enabled = !!(buf & 0x01);

	success = true;
RETURN:
	return success;
}

bool pcf8523_bl_int_enable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf |= 0x01;

	if (!pcf8523_control_reg_set(i2c_inst, 3, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_bl_int_disable(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_control_reg_get(i2c_inst, 3, &buf))	
		goto RETURN;

	buf &= ~0x01;

	if (!pcf8523_control_reg_set(i2c_inst, 3, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_ci_warning_flag_is_set(uint i2c_inst, bool *is_set) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_seconds_reg_get(i2c_inst, &buf))	
		goto RETURN;

	*is_set = !!(buf & 0x80);

	success = true;
RETURN:
	return success;
}

bool pcf8523_ci_warning_flag_clear(uint i2c_inst) {
	bool success = false;

	uint8_t buf;
	if (!pcf8523_seconds_reg_get(i2c_inst, &buf))	
		goto RETURN;

	buf &= ~0x80;

	if (!pcf8523_seconds_reg_set(i2c_inst, buf))	
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_time_date_reg_get_all(uint i2c_inst, uint8_t dst[7]) {
	bool success = false;

	uint8_t reg = PCF8523_REG_SECONDS;
	if (!pcf8523_i2c_write(i2c_inst, &reg, 1))
		goto RETURN;	

	if (!pcf8523_i2c_read(i2c_inst, dst, 7))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_time_date_reg_set_all(uint i2c_inst, uint8_t src[7]) {
	bool success = false;

	uint8_t buf[8] = {PCF8523_REG_SECONDS};
	for (int i = 0; i < 7; i++)
		buf[i+1] = src[i];

	if (!pcf8523_i2c_write(i2c_inst, buf, 8))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_time_reg_get_all(uint i2c_inst, uint8_t dst[3]) {
	bool success = false;

	uint8_t reg = PCF8523_REG_SECONDS;
	if (!pcf8523_i2c_write(i2c_inst, &reg, 1))
		goto RETURN;	

	if (!pcf8523_i2c_read(i2c_inst, dst, 3))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_time_reg_set_all(uint i2c_inst, uint8_t src[3]) {
	bool success = false;

	uint8_t buf[8] = {PCF8523_REG_SECONDS};
	for (int i = 0; i < 3; i++)
		buf[i+1] = src[i];

	if (!pcf8523_i2c_write(i2c_inst, buf, 3))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_seconds_reg_get(uint i2c_inst, uint8_t *dst) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_SECONDS, dst))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_seconds_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_SECONDS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_seconds_get(uint i2c_inst, uint *seconds) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_seconds_reg_get(i2c_inst, &buf))
		goto RETURN;	

	*seconds = 0;
	*seconds += buf & 0x0F;
	*seconds += ((buf & 0x70) >> 4) * 10;

	success = true;
RETURN:
	return success;
}

bool pcf8523_seconds_set(uint i2c_inst, uint seconds) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_seconds_reg_get(i2c_inst, &buf))
		goto RETURN;	

	// Clear all but ci flag
	buf &= 0x80;
	buf |= seconds % 10;
	buf |= ((seconds / 10) << 4) & 0x70;

	if (!pcf8523_seconds_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_minutes_reg_get(uint i2c_inst, uint8_t *reg) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_MINUTES, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_minutes_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_MINUTES, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_minutes_get(uint i2c_inst, uint *minutes) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_minutes_reg_get(i2c_inst, &buf))
		goto RETURN;	

	*minutes = 0;
	*minutes += buf & 0x0F;
	*minutes += ((buf & 0x70) >> 4) * 10;

	success = true;
RETURN:
	return success;
}

bool pcf8523_minutes_set(uint i2c_inst, uint minutes) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_minutes_reg_get(i2c_inst, &buf))
		goto RETURN;	

	buf &= 0x80;
	buf |= minutes % 10;
	buf |= ((minutes / 10) << 4) & 0x70;

	if (!pcf8523_minutes_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_hours_reg_get(uint i2c_inst, uint8_t *reg) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_HOURS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_hours_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_HOURS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_hours_get(uint i2c_inst, uint *hours) {
	bool success = false;

	HOUR_MODE_T hm;
	if (!pcf8523_hour_mode_get(i2c_inst, &hm))
		goto RETURN;

	uint8_t buf = 0;
	if (!pcf8523_hours_reg_get(i2c_inst, &buf))
		goto RETURN;

	*hours = 0;
	*hours += buf & 0x0F;

	switch (hm) {
	case HOUR_MODE_24:
		*hours += ((buf & 0x30) >> 4) * 10;
		break;
	case HOUR_MODE_12:
		*hours += ((buf & 0x10) >> 4) * 10;
		break;
	}

	success = true;
RETURN:
	return success;
}

bool pcf8523_hours_set(uint i2c_inst, uint hours) {
	bool success = false;

	HOUR_MODE_T hm;
	if (!pcf8523_hour_mode_get(i2c_inst, &hm))
		goto RETURN;

	uint8_t buf = 0;
	if (!pcf8523_hours_reg_get(i2c_inst, &buf))
		goto RETURN;

	switch (hm) {
	case HOUR_MODE_24:
		buf &= ~0x3F;
		buf |= ((hours / 10) << 4) & 0x30;
		break;
	case HOUR_MODE_12:
		buf &= ~0x1F;
		buf |= ((hours / 10) << 4) & 0x10;
		break;
	}

	buf |= hours % 10;

	if (!pcf8523_hours_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

// typedef enum _AM_PM_E {
// 	AM = 0,
// 	PM = 1
// } AM_PM_T;
bool pcf8523_am_pm_get(uint i2c_inst, AM_PM_T *am_pm) {
	bool success = false;

	HOUR_MODE_T hm;
	if (!pcf8523_hour_mode_get(i2c_inst, &hm))
		goto RETURN;

	switch (hm) {
	case HOUR_MODE_24:
		uint hours;
		if (!pcf8523_hours_get(i2c_inst, &hours))
			goto RETURN;

		if (hours < 12) *am_pm = AM;
		else *am_pm = PM;

		break;

	case HOUR_MODE_12:
		uint8_t buf = 0;
		if (!pcf8523_hours_reg_get(i2c_inst, &buf))
			goto RETURN;

		if (buf & 0x20) *am_pm = PM;
		else *am_pm = AM;

		break;
	}

	success = true;
RETURN:
	return success;
}

bool pcf8523_am_pm_set(uint i2c_inst, AM_PM_T am_pm) {
	bool success = false;

	HOUR_MODE_T hm;
	if (!pcf8523_hour_mode_get(i2c_inst, &hm))
		goto RETURN;

	switch (hm) {
	case HOUR_MODE_24:
		uint hours;
		if (!pcf8523_hours_get(i2c_inst, &hours))
			goto RETURN;

		if (am_pm == AM && hours >= 12)
			hours -= 12;
		else if (am_pm == PM && hours < 12)
			hours += 12;

		if (!pcf8523_hours_set(i2c_inst, hours))
			goto RETURN;

		break;

	case HOUR_MODE_12:
		uint8_t buf = 0;
		if (!pcf8523_hours_reg_get(i2c_inst, &buf))
			goto RETURN;

		buf &= ~0x20;
		buf |= (am_pm & 0x01) << 5;

		if (!pcf8523_hours_reg_set(i2c_inst, buf))
			goto RETURN;

		break;
	}

	success = true;
RETURN:
	return success;
}

bool pcf8523_days_reg_get(uint i2c_inst, uint8_t *reg) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_DAYS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_days_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_DAYS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_days_get(uint i2c_inst, uint *days) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_days_reg_get(i2c_inst, &buf))
		goto RETURN;	

	*days = 0;
	*days += buf & 0x0F;
	*days += ((buf & 0x30) >> 4) * 10;

	success = true;
RETURN:
	return success;
}

bool pcf8523_days_set(uint i2c_inst, uint days) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_days_reg_get(i2c_inst, &buf))
		goto RETURN;	

	buf &= ~0x3F;
	buf |= days % 10;
	buf |= ((days / 10) << 4) & 0x30;

	if (!pcf8523_days_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

// typedef enum _WEEKDAY_E {
// 	SUNDAY,
// 	MONDAY,
// 	TUESDAY,
// 	WEDNESDAY,
// 	THURSDAY,
// 	FRIDAY,
// 	SATURDAY
// } WEEKDAY_T;
bool pcf8523_weekdays_reg_get(uint i2c_inst, uint8_t *reg) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_WEEKDAYS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_weekdays_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_WEEKDAYS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_weekdays_get(uint i2c_inst, WEEKDAY_T *weekday) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_weekdays_reg_get(i2c_inst, &buf))
		goto RETURN;	

	*weekday = 0;
	*weekday += buf & 0x07;

	success = true;
RETURN:
	return success;
}
	
bool pcf8523_weekdays_set(uint i2c_inst, WEEKDAY_T weekday) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_weekdays_reg_get(i2c_inst, &buf))
		goto RETURN;	

	buf &= ~0x07;
	buf |= weekday & 0x07;

	if (!pcf8523_weekdays_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

// typedef enum _MONTH_E {
// 	JANUARY,
// 	FEBRUARY,
// 	MARCH,
// 	APRIL,
// 	MAY,
// 	JUNE,
// 	JULY,
// 	AUGUST,
// 	SEPTEMBER,
// 	OCTOBER,
// 	NOVEMBER,
// 	DECEMBER
// } MONTH_T;
bool pcf8523_months_reg_get(uint i2c_inst, uint8_t *reg) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_MONTHS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_months_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_MONTHS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_months_get(uint i2c_inst, MONTH_T *month) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_months_reg_get(i2c_inst, &buf))
		goto RETURN;	

	*month = 0;
	*month += buf & 0x0F;
	*month += ((buf & 0x10) >> 4) * 10;

	success = true;
RETURN:
	return success;
}

bool pcf8523_months_set(uint i2c_inst, MONTH_T month) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_months_reg_get(i2c_inst, &buf))
		goto RETURN;	

	buf &= ~0x3F;
	buf |= month % 10;
	buf |= ((month / 10) << 4) & 0x10;

	if (!pcf8523_months_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}

bool pcf8523_years_reg_get(uint i2c_inst, uint8_t *reg) {
	bool success = false;

	if (!pcf8523_reg_get(i2c_inst, PCF8523_REG_YEARS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}
bool pcf8523_years_reg_set(uint i2c_inst, uint8_t reg) {
	bool success = false;

	if (!pcf8523_reg_set(i2c_inst, PCF8523_REG_YEARS, reg))
		goto RETURN;	

	success = true;
RETURN:
	return success;
}

bool pcf8523_years_get(uint i2c_inst, uint *years) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_years_reg_get(i2c_inst, &buf))
		goto RETURN;	

	*years = 0;
	*years += buf & 0x0F;
	*years += ((buf & 0xF0) >> 4) * 10;

	success = true;
RETURN:
	return success;
}

bool pcf8523_years_set(uint i2c_inst, uint years) {
	bool success = false;

	uint8_t buf = 0;
	if (!pcf8523_years_reg_get(i2c_inst, &buf))
		goto RETURN;	

	buf &= 0x00;
	buf |= years % 10;
	buf |= ((years / 10) << 4) & 0xF0;

	if (!pcf8523_years_reg_set(i2c_inst, buf))
		goto RETURN;

	success = true;
RETURN:
	return success;
}
