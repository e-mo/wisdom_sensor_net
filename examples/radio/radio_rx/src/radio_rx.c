//  rfm69_dio_rx
//  2025.02.04

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

// rfm69 packet rx/tx example utilizing gio interrupts.

#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

#include "whale.h"

void main() {
	stdio_init_all();
	while (!tud_cdc_connected()) { sleep_ms(100); };

	if (whale_init() == false)
		goto ERROR_LOOP;

	whale_radio_node_address_set(0x01);
#define PAYLOAD_BUFFER_SIZE (1024 * 5)
	for (;;) {
		uint8_t buffer[PAYLOAD_BUFFER_SIZE] = {0};
		uint32_t received;

		if (!whale_radio_recv(buffer, PAYLOAD_BUFFER_SIZE, &received))
			printf("Rx failed\n");
		else {
			printf("Received: %u\n", received);
		}
	}

ERROR_LOOP:
	for (;;) {
		printf("HAL failed to initialize.\n");
		sleep_ms(3000);
	}
}
