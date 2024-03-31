#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "gateway_interface.h"
#include "gateway_queue.h"
#include "gateway_error.h"
#include "gw_core.h"
#include "gw_core_error.h"

#include "gateway_codes.h" // Communication codes


void gateway_init(void) {
	gw_queue_init();

	multicore_launch_core1(gw_core_entry);
}

void gateway_start(void) {

	uint32_t command[2];
	command[0] = GATEWAY_COMMAND;
	command[1] = GATEWAY_START;
	gw_queue_main_send((uint8_t *)command, (sizeof (uint32_t)) * 2);

}

void gateway_stop(void) {
	uint32_t command[2];
	command[0] = GATEWAY_COMMAND;
	command[1] = GATEWAY_STOP;
	gw_queue_main_send((uint8_t *)command, (sizeof (uint32_t)) * 2);
}

void gateway_send(void *data, uint size) {
	
}


int test_gateway() {
	return 1;
//    stdio_init_all(); // To be able to use printf
//	
//	bool success = false;
//	char *error_str = "ok";
//					  
//	Modem *modem = modem_start(
//			MODEM_APN,
//			UART_PORT,
//			UART_PIN_TX,
//			UART_PIN_RX,
//			MODEM_PIN_PWR
//	);
//
//	if (modem == NULL) {
//		error_str = "modem_start returned NULL";
//		goto LOOP_BEGIN;
//	}
//
//
//	success = modem_cn_activate(modem, true)) printf("Network activated\n");
//
//	success = true;
//LOOP_BEGIN:
//	if (!success)
//		for (;;)
//			printf("Error: %s\n", error_str); 
//			sleep_ms(3000);
//		}
//
//	for(;;) {
//
//		printf("Modem init success!\n"); 
//		sleep_ms(3000);
//	}
//
//    
//    return 0;
}
