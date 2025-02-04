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
#include "pico/stdlib.h"
//#include "tusb.h"

#include "radio.h"

void error_loop(char *error) {
	for (;;) {
		printf("Error: %s\n", error);
		sleep_ms(3000);
	}
}

int main() {
    stdio_init_all(); // To be able to use printf
	
	bool success = false;
	
	if (radio_init() == false)
		goto LOOP_BEGIN;	

	success = true;

LOOP_BEGIN:;

	char status_str[ERROR_STR_MAX];
	if (success = false) {
		radio_status(status_str);		
		error_loop(status_str);
	}

	radio_address_set(0x02);

#define BUFFER_SIZE (100)
	char buffer[BUFFER_SIZE] = {0};

	int i = 1;
	uint received = 0;
	for (;;) {

		success = radio_recv(buffer, BUFFER_SIZE, &received);

		printf("#%i\n", i);

		if (!success) {
			radio_status(status_str);		
			printf("radio_recv failed:\n%s\n", status_str);
		} else 
			printf("message received (%u B): %s\n\n", received, buffer);
		
		i++;
	}
    
    return 0;
}
