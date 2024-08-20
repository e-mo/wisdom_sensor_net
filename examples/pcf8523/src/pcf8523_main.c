#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "pcf8523_rp2040.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (9)
#define PIN_SDA  (8)

#define ever ;;

void main() {
	stdio_init_all();

	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);

	uint index = I2C_NUM(I2C_INST);

	uint buf = 69;

	pcf8523_software_reset_initiate(index);

	pcf8523_cap_sel_set(index, CAP_SEL_12_5_PF);
	pcf8523_correction_int_enable(index);
	//pcf8523_correction_int_disable(index);

	for(ever) {

		uint8_t buf = 0x00;
		if (!pcf8523_control_get(index, 1, &buf))
			printf("Unabled to read\n");

		printf("buf: %02X\n", buf);

		bool is_enabled = true;
		if (!pcf8523_correction_int_is_enabled(index, &is_enabled))
			printf("Unabled to read\n");

		printf("%s\n", is_enabled ? "true" : "false");

		sleep_ms(1000);

	}
}

