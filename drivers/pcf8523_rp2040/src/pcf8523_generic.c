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
bool pcf8523_hour_mode_get(uint i2c_inst, uint *hour_mode) {
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
