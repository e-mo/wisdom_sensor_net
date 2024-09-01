#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "pcf8523_rp2040.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

#define ever ;;

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

	for(ever) {

		uint8_t buf = 0;
		if (!pcf8523_seconds_reg_get(index, &buf))
			printf("Unabled to read\n");

		printf("buf: %0X\n", buf);
		pcf8523_ci_warning_flag_clear(index);

		sleep_ms(1000);

	}
}

