// template_main.c

//	Copyright (C) 2024 
//	Evan Morse
//	Amelia Vlahogiannis
//	Noelle Steil
//	Jordan Allen
//	Sam Colwell
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
#include "pico/stdlib.h"
#include "tusb.h"

// OPTIONAL INCLUDES
// Uncomment depending on libraries used

// RFM69 library
//#include "rfm69_pico.h" 

// SD card library
#include "sd_config.h"
#include "f_util.h"

void error_loop(char *error) {
	for (;;) {
		printf("SD error: %s\n", error);
		sleep_ms(3000);
	}
}

int main() {
    stdio_init_all(); // To be able to use print

	bool success = false;
	char error[100] = "success";

	while (!tud_cdc_connected()) { sleep_ms(100); };
	// Get SD config (ref sd_config.c)  
	sd_card_t *sd = sd_get_by_num(0);
	if (sd == NULL) {
		sprintf(error, "sd_get_by_num returned NULL");
		goto LOOP_BEGIN;
	}

	FRESULT fr = f_mount(&sd->fatfs, sd->pcName, 1);
	if (fr != FR_OK) {
		sprintf(error, "f_mount: %s (%d)", FRESULT_str(fr), fr);
		goto LOOP_BEGIN;
	}

	success = true;
LOOP_BEGIN:
	if (!success) error_loop(error);

	int i = 1;
	for(;;) {
		// Wait for USB serial connection

		printf("%i: Hello, world!", i);

		i++;
		sleep_ms(1000);
	}
    
    return 0;
}
