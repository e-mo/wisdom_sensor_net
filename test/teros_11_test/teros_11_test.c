#include <stdio.h>
#include "pico/stdlib.h"
#include "teros_11.h"

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define TEROS_PWR_PIN 7


int main()
{
    stdio_init_all();
    teros *teros;
    teros_init(
		    &teros,
		    uart0,
		    teros_11,
		    UART_TX_PIN,
		    UART_RX_PIN,
		    TEROS_PWR_PIN,
		    soilless
	      );
		    

    puts("Hello, world!");

    return 0;
}
