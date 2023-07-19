#include <stdio.h>

#define ever ;;

void modem_core_main(void) {
	
	printf("Modem core started!\n");

	for(ever) {
		sleep_ms(2000);
	}
}
