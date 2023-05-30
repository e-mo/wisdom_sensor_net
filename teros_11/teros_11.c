/*
 * teros_11.c
 * 2023-05-29
 * amelia vlahogiannis
 */

struct teros_11 {
	uart_inst_t *serial;


struct teros_data {
	int calibrated_counts_vwc;
	float temp;
	unsigned char sensor_type;
	unsigned char checksum;
};

teros_return teros_11_init(
		&teros,
		uart_inst_t *serial,
		uart_tx_pin,
		uart_rx_pin,
		int teros_pwr_pin) {
	if(uart_is_enabled(serial)) return uart_not_available;

}

teros_return teros_11_read(teros_11 *teros, ) {
	//power on sensor
	//while(!data_pin); //until data pin is set high
	//wait for falling edge to start reading serial data
	//convert some bytes to ints and floats and stuff
	//store in that one struct
}
