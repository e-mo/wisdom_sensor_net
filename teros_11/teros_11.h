/*
 * pi pico library for the Teros 11 soil moisture sensor
 * uses "DDI Serial" (aka uart but they refuse to just call it that)
 * because the other option is some weird standard that appears to just be rs232?
 *
 * 2023-05-29
 * amelia vlahogiannis
 */

//serial port is 1200baud

#ifndef TEROS_11_H
#define TEROS_11_H

#include "pico/stdlib.h"

typedef enum teros_return {
	ok			=  0,
	uart_already_enabled	= -1;
}

typedef enum teros_model {
	teros_11,
	teros_12;
}

typedef enum teros_substrate {
	mineral,
	soilless;
}

typedef struct teros {
	uart_inst_t serial;
	teros_model model;
	int uart_tx_pin;
	int uart_rx_pin;
	int pwr_pin;
	teros_substrate_type substrate_type;
}

teros_return teros_get_checksum(teros *teros, uint8_t *checksum);
teros_return teros_get_crc(teros *teros, uint8_t *crc);
teros_return teros_get_sensor_type(teros *teros, uint8_t *type);
teros_return teros_get_vwc(teros *teros, float *vwc);
teros_return teros_get_vwc_raw(teros *teros, float *vwc);
teros_return teros_get_permittivity(teros *teros, float *permittivity);
teros_return teros_get_tempc(teros *teros, float *temp);

float raw_to_m3m3_mineral(float raw);
float raw_to_m3m3_soilless(float raw);

#endif 
