#include <stdio.h>
#include "pico/stdlib.h"

#include "hibernate.h"

int main() {
	stdio_init_all();

	hibernate_run_from_dormant_source(DORMANT_SOURCE_XOSC);

	hibernate_goto_dormant_until_pin(2, false, false);

	for (;;) {

		printf("hello!\n");
		sleep_ms(1000);

	}
}
