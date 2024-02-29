#include "teros_11.h"

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define TEROS_PWR_PIN 3


int main()
{

	stdio_init_all(); //for debugging purposes
	teros *teros; //name our teros sensor struct
	teros_data t11_data; //struct to hold our sensor readings

	sleep_ms(2000);

	teros_init( //initialize it!
		    &teros,
		    uart0, //pick a uart
		    teros_11, //teros_11 or teros_12
		    UART_TX_PIN,
		    UART_RX_PIN,
		    TEROS_PWR_PIN,
		    soilless //soilless or mineral
		);

	sleep_ms(3000);

	while(1) {
		printf("\nline\n");
		if(teros_get_data(teros, &t11_data)) printf("ERROR\n");
		printf("%f", t11_data.vwc);
		sleep_ms(1000);
	}
    return 0;
}
