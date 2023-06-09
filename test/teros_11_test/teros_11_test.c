#include "teros_11.h"

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define TEROS_PWR_PIN 3

#define ever ;;


int main()
{

	stdio_init_all(); //for debugging purposes
	teros *teros; //name our teros sensor struct
	teros_data t11; //struct to hold our sensor readings

	sleep_ms(2000);

	teros_init( //initialize it!
		    &teros,
		    uart0, //pick a uart
		    teros_11, //teros_11 or teros_12
		    UART_TX_PIN,
		    UART_RX_PIN,
		    TEROS_PWR_PIN,
		    mineral //soilless or mineral
		);

	sleep_ms(3000);

	for(ever) {
		if(teros_get_data(teros, &t11)) {
			printf("ERROR\n");
		}

		printf("Soil moisture:  %.6f m3/m3\n", t11.vwc);
		printf("Temperature:    %.1f C\n", t11.temperature);
		printf("Sensor type:    %c\n", t11.sensor_type);
		printf("Checksum:       %c\n", t11.checksum);
		printf("CRC:            %c\n", t11.crc);
		printf("\n");

		sleep_ms(1000);
	}
    return 0;
}
