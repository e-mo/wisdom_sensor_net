#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"

#include "hibernate.h"
#include "pcf8523_rp2040.h"

#include "radio.h"

#define I2C_INST (i2c0)
#define PIN_SCL  (5)
#define PIN_SDA  (4)

extern bool usb_clock_stopped;

void gpio_callback_alarm(uint pin, uint32_t event) {
	printf("event!\n");
}

int main() {
	// Radio
	radio_init();	
	radio_address_set(0x01);


	// Setup RTC to send interrupt soon
	i2c_init(I2C_INST, 500 * 1000);
	gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
	gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
	gpio_pull_up(PIN_SCL);
	gpio_pull_up(PIN_SDA);
	gpio_pull_up(2); // Pull up irq pin

	uint index = I2C_NUM(I2C_INST);

	pcf8523_software_reset_initiate(index);
	// Disable clockout (must do before setting up interrupt)
	pcf8523_clockout_freq_set(index, CLOCKOUT_DISABLED);


	// Attempt to print after waking up (proving USB serial is OK)
	bool light = false;
	for (;;) {

		pcf8523_seconds_set(index, 55);

		pcf8523_minutes_set(index, 0);
		pcf8523_minute_alarm_set(index, 1);
		pcf8523_minute_alarm_enable(index);
		pcf8523_alarm_int_flag_clear(index);

		pcf8523_alarm_int_enable(index);

		// Save old clocks
		uint clock0_orig = clocks_hw->sleep_en0;
		uint clock1_orig = clocks_hw->sleep_en1;

		// Configure clocks to go dormant
		hibernate_run_from_dormant_source(DORMANT_SOURCE_XOSC);
		char payload[6];
		snprintf(message, 6, "%s", usb_clock_stopped ? "true" : "false"); 
		uint payload_len = strlen(payload) + 1;

		// Go dormant until falling edge of active low signal
		//gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback_alarm);
		hibernate_goto_dormant_until_pin(2, true, false);

		// Recover clocks after hibernation
		hibernate_recover_clocks(clock0_orig, clock1_orig);

		radio_send(payload, payload_len, 0x02);

		//uint seconds;
		//pcf8523_seconds_get(index, &seconds);	
		//printf("%u\n", seconds);
		//printf("hello\n");
		sleep_ms(1000);

	}
}
