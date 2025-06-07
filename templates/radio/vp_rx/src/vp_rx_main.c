// radio_rx.c

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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "whale.h"
#include "rfm69_vp.h"
#include "wtp-1_0.h"

extern rfm69_context_t rfm69_ctx;

void main() {
	stdio_init_all();
	while (!tud_cdc_connected()) { sleep_ms(100); };
	
	int rval = whale_init(W_RADIO_MODULE);
	if (rval != WHALE_OK)
		goto ERROR_LOOP;

	w_radio_node_address_set(0x01);
	//w_radio_dbm_set(20);

#define PAYLOAD_BUFFER_SIZE (200)
	uint8_t buffer[PAYLOAD_BUFFER_SIZE] = {0};
	int received = {0};
	int tx_addr = {0};
	int rssi = 0;
	for (;;) {

		int rval = rfm69_vp_rx(
				&rfm69_ctx,
				buffer,
				PAYLOAD_BUFFER_SIZE,
				5000,
				&rssi
		);

		if (rval == VP_RX_OK) {
			printf("Received\n");
		} else {
			switch (rval) {
			case VP_RX_TIMEOUT:
				printf("Timeout\n");
				break;
			case VP_RX_BUFFER_OVERFLOW:
				printf("Buffer overflow\n");
				break;
			case VP_RX_CRC_FAILURE:
				printf("def\n");
				break;
			}
		}

		sleep_ms(1000);
	}

	// Loop forever with error
ERROR_LOOP:
	for (;;) {
		printf("Radio module failed to initialize: ");
		printf("%u\n", w_module_state_query(W_RADIO_MODULE));
		sleep_ms(3000);
	}
}
