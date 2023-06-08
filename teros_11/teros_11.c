/*
 * teros_11.c
 * 2023-05-29
 * amelia vlahogiannis
 */

#include "teros_11.h"

teros_return teros_init(
		teros **teros, 
		uart_inst_t *serial, 
		teros_model model,
		int uart_tx_pin,
		int uart_rx_pin,
		int pwr_pin,
		teros_substrate substrate_type
		) {

	*teros = malloc(sizeof(teros));

	//save some struct elements to reference in other functions
	(*teros)->serial = serial;
	(*teros)->model = model;
	(*teros)->uart_tx_pin = uart_tx_pin;
	(*teros)->uart_rx_pin = uart_rx_pin;
	(*teros)->pwr_pin = pwr_pin;
	(*teros)->substrate_type = substrate_type;

	uart_init(serial, TEROS_BAUD_RATE);

	gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
	//gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
	gpio_init(pwr_pin);
	gpio_set_dir(pwr_pin, GPIO_OUT);
}

//grab and return all of the data
teros_return teros_get_data(teros *teros, teros_data *data) {
	teros_return status;
	unsigned char ch;
	unsigned char str[16];
	int i = 0;

	printf("get data function start \n");
	
	gpio_put(teros->pwr_pin, 1);

	printf("gpio on\n");
	
	if(!uart_is_enabled(teros->serial)) return uart_not_enabled;

	sleep_ms(125);

	printf("uart is enabled\n");

	if(uart_is_readable_within_us(teros->serial, 150000)) {
		i = 0;
		while(uart_is_readable(uart0)) {
				str[i] = uart_getc(uart0);
				i++;
				}
		str[i] = '\0';
		printf("%s\n", str);
		printf("reading complete\n");
		}
	
	else printf("could not read within 150ms\n");
	
	gpio_put(teros->pwr_pin, 0);

	sleep_ms(3000);

	return 0;
}

//get checksum
teros_return teros_get_checksum(teros *teros, uint8_t *checksum) {
	gpio_put(teros->pwr_pin, 1); //sensor will automatically spit out data over uart when powered on
				     //the pico's digital pins can supply the necessary 3.6mA
}

//get crc
teros_return teros_get_crc(teros *teros, uint8_t *crc) {
}

//get sensor type
teros_return teros_get_sensor_type(teros *teros, uint8_t *type) {
}

//get vwc in m3/m3
teros_return teros_get_vwc(teros *teros, float *vwc) {
}

//get vwc raw
teros_return teros_get_vwc_raw(teros *teros, float *vwc) {
}

//get dielectric permittivity - teros 12 only
teros_return teros_get_permittivity(teros *teros, float *permittivity) {
}

//get temp in c
teros_return teros_get_tempc(teros *teros, float *temp) {
}

//convert raw to m3/m3 for mineral soils
float raw_to_m3m3_mineral(float raw) {
	float vwc;
	vwc = 0.0003879 * raw - 0.6956;
	return vwc;
}

//convert raw to m3/m3 for soilless media
float raw_to_m3m3_soilless(float raw) {
	float vwc;
	vwc = 0.0000000006771 * (raw * raw * raw) - 0.000005105 * (raw * raw) + 0.01302 * raw - 10.848;
	return vwc;
}

