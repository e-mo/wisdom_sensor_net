/*
 * teros_11.c
 * 2023-05-29
 * amelia vlahogiannis
 */


int teros_init(
		teros *teros,
		uart_inst_t serial,
		teros_model model,
		int uart_tx_pin,
		int uart_rx_pin,
		int pwr_pin,
		teros_substrate substrate_type
		) {
	
	*teros->serial = serial;
	*teros
	uart_init(*serial, 1200);
	gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
	gpio_set_function(uart_tx_pin, GPIO_FUNC_UART);
	gpio_init(pwr_pin);
	gpio_set_dir(pwr_pin, GPIO_OUT);
}

//get checksum
teros_return teros_get_checksum(teros *teros, uint8_t *checksum) {
	gpio_put(teros->pwr_pin, 1);
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

