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

#include "gateway.h"
#include "rp2x_rfm69_rudp.h"

rfm69_context_t rfm;

void collect_and_send(void) {
	uint8_t buf[100] = {0};
	rfm69_rx_variable_packet(&rfm, buf, 100, 10000);

	gateway_queue_push(buf, 100);

PUMP:;
	int rval = gateway_pump();
	
	while (rval != MODEM_POWERED_DOWN) {
		switch (rval) {
		case MODEM_POWERED_DOWN:
			printf("State: MODEM_POWERED_DOWN");
			break;
		case MODEM_STOPPED:
			printf("State: MODEM_STOPPED");
			break;
		case MODEM_STARTED:
			printf("State: MODEM_STARTED");
			break;
		case MODEM_CN_ACTIVE:
			printf("State: MODEM_CN_ACTIVE");
			break;
		case MODEM_SERVER_CONNECTED:
			printf("State: MODEM_SERVER_CONNECTED");
			break;
		}

		sleep_ms(500);
		rval = gateway_pump();
	}

	printf("farted!");
}

int main() {
	stdio_init_all();
	// Wait for USB serial connection
	//while (!tud_cdc_connected()) { sleep_ms(100); };
	//
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

	rfm69_node_address_set(&rfm,0x00);

	// Gate init
	if (!gateway_init()) {
		printf("gateway_init fail");
		goto IDLE_LOOP;
	}

	for (;;) {
		collect_and_send();
		sleep_ms(1000);
	}
IDLE_LOOP:
	for (;;) sleep_ms(1000);
    
    return 0;
}
