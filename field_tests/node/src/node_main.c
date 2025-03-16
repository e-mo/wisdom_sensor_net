// template_main.c

//	Copyright (C) 2024 
//	Evan Morse
//	Amelia Vlahogiannis
//	Noelle Steil
//	Jordan Allen
//	Sam Cowan
//	Rachel Cleminson

//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.

//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.

//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.

//#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
//#include "tusb.h"

//#include "gateway.h"
#include "rp2x_rfm69_rudp.h"

rfm69_context_t rfm;

void send_reading(void) {

	static uint8_t buf[100] = {0};

	// read into buf here and send

	rfm69_tx_variable_packet(&rfm, 0x00, buf, 100);
}

int main() {
	// SPI init
    spi_init(RFM69_SPI, 1000*1000);
    gpio_set_function(RFM69_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(RFM69_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(RFM69_PIN_MOSI, GPIO_FUNC_SPI);

	// Drive CS pin high
    gpio_init(RFM69_PIN_CS);
    gpio_set_dir(RFM69_PIN_CS, GPIO_OUT);
    gpio_put(RFM69_PIN_CS, 1);

	struct rfm69_config_s config = {
		.spi      = RFM69_SPI,
		.pin_cs   = RFM69_PIN_CS,
		.pin_rst  = RFM69_PIN_RST,
		.pin_dio0 = RFM69_PIN_DIO0,
		.pin_dio1 = RFM69_PIN_DIO1,
		.pin_dio2 = RFM69_PIN_DIO2
	};

	rfm69_context_t rfm;
	if (rfm69_rudp_init(&rfm, &config) == false)
		goto IDLE_LOOP;

	rfm69_node_address_set(&rfm,0x01);

	for (;;) {
		// Call send_reading every 10secs
		send_reading();
		sleep_ms(10000);
	}
IDLE_LOOP:
	for (;;) sleep_ms(1000);
    
    return 0;
}
