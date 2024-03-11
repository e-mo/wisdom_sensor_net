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

#include "radio.h"

// OPTIONAL INCLUDES
// Uncomment depending on libraries used

// RFM69 library
//#include "rfm69_pico.h" 

// SD card library
//#include "sd_config.h"

int main() {
    stdio_init_all(); // To be able to use printf
	
	bool success = false;
	char status_str[ERROR_STR_MAX] = "success";
	
	// Wait for USB serial connection
	while (!tud_cdc_connected()) { sleep_ms(100); };

	if (radio_init() == false)
		goto LOOP_BEGIN;	


	success = true;
LOOP_BEGIN:
	radio_status(status_str);		

	int i = 1;
	for(;;) {

		printf("%i: success=%s\n", i, success ? "true" : "false");
		printf("status=%s\n", status_str);	

		i++;
		sleep_ms(1000);
	}
    
    return 0;
}
