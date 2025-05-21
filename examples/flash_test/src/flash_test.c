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
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "tusb.h"

#include "wisdom_hal.h"

extern uint32_t ADDR_PERSISTENT[];

void main() {
	stdio_init_all();
	while (!tud_cdc_connected()) { sleep_ms(100); };

	if (whal_init() == false)
		goto ERROR_LOOP;

	uint32_t buf[FLASH_PAGE_SIZE/sizeof(uint32_t)];
	buf[0] = 0x45;

	uint32_t address = (uint32_t)ADDR_PERSISTENT;
	uint32_t flash_address = (address - XIP_BASE);

	uint32_t ints = save_and_disable_interrupts();
	//flash_range_program(flash_address, (uint8_t *)buf, FLASH_PAGE_SIZE);
	uint32_t out = *(uint32_t *)address;
	restore_interrupts(ints);

	for (;;) {
		printf("address: 0x%X\n", address);
		printf("value: 0x%X\n", out);
		sleep_ms(1000);
	}

ERROR_LOOP:
	for (;;) {
		printf("HAL failed to initialize.\n");
		sleep_ms(3000);
	}
}
