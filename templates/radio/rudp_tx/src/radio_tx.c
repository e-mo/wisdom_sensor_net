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

void main() {
	stdio_init_all();
	//while (!tud_cdc_connected()) { sleep_ms(100); };
	
	int rval = whale_init(W_RADIO_MODULE);
	if (rval != WHALE_OK)
		goto ERROR_LOOP;

#define PAYLOAD_SIZE (1024 * 4)
	uint8_t payload[PAYLOAD_SIZE];
	for (int i = 0; i < PAYLOAD_SIZE; i++)
		payload[i] = i;

	//w_radio_dbm_set(20);

	int rssi = 0;
	int rtr = 0;
	for (;;) {
		
		if (w_radio_tx(0x01, payload, PAYLOAD_SIZE) != W_RADIO_OK)
			printf("Tx failure\n");
		else {
			printf("Tx success\n");
			w_radio_rssi_get(&rssi);
			w_radio_rtr_count_get(&rtr);
			printf("RSSI: %i\n", rssi);
			printf("rtr: %i\n", rtr);
		}

		sleep_ms(20);
	}

	// Loop forever with error
ERROR_LOOP:
	for (;;) {
		printf("Radio module failed to initialize: ");
		printf("%u\n", w_module_state_query(W_RADIO_MODULE));
		sleep_ms(3000);
	}
}
