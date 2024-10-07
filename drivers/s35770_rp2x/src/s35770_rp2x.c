#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "s35770_rp2x.h"

void s35770_rp2x_init(s35770_context_t *counter, uint i2c_inst, uint pin_reset) {

	// init reset pin
	gpio_set_function(pin_reset, GPIO_FUNC_SIO);
	gpio_pull_up(pin_reset);	
	gpio_put(pin_reset, 1);

	counter->pin_reset = pin_reset;
	counter->i2c_inst = i2c_inst;
}

void s35770_rp2x_reset(s35770_context_t *counter) {
	gpio_put(counter->pin_reset, 0);
	sleep_ms(1000);
	gpio_put(counter->pin_reset, 1);
}

bool s35770_rp2x_read(s35770_context_t *counter, uint32_t *count) {
	uint8_t buf[3] = {0};	
	int rval = i2c_read_blocking(
			I2C_INSTANCE(counter->i2c_inst),
			S35770_I2C_ADDRESS,
			buf, 
			3,
			false
	);	

	if (rval != 3) return false;

	*count = 0;	
	*count |= buf[2];
	*count |= buf[1] << 8;
	*count |= buf[0] << 16;

	return true;
}
