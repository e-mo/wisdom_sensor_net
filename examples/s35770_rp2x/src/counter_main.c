#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "s35770_rp2x.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (1)
#define PIN_SDA  (0)
#define PIN_RESET (2)

#define ever ;;

void main() {
	stdio_init_all();

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);

	uint index = I2C_NUM(I2C_INST);

	s35770_context_t counter;

	s35770_rp2x_init(&counter, index, PIN_RESET);
	s35770_rp2x_reset(&counter);

	uint32_t count = 0;
	for(int i = 0;; i++) {
		s35770_rp2x_read(&counter, &count);
		printf("%u\n", count);
		sleep_ms(1000);
	}
}

