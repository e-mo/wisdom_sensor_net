/*
 * pi pico library for the Teros 11 soil moisture sensor
 * uses "DDI Serial" (aka uart but they dont call it that)
 * because the other option is some weird standard that appears to just be rs232?
 *
 * 2023-05-29
 * amelia vlahogiannis
 * amelia@ag2v.com
 */

//serial port is 1200baud

#ifndef TEROS_11_H
#define TEROS_11_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/malloc.h"
#define TEROS_BAUD_RATE 1200

typedef enum _return {
	ok			=  0,
	invalid_check_crc	= -1,
	uart_not_enabled	= -2,
} teros_return;

typedef enum _model {
	teros_11,
	teros_12,
} teros_model;

typedef enum _substrate {
	mineral,
	soilless,
} teros_substrate;

typedef struct _teros {
	uart_inst_t *serial;
	teros_model model;
	int uart_tx_pin;
	int uart_rx_pin;
	int pwr_pin;
	teros_substrate substrate_type;
} teros;

//<TAB><calibratedCountsVWC> <temperature> <electricalConductivity><CR><sensorType><Checksum><CRC>
typedef struct _data {
	float vwc;
	float temperature;
	float conductivity;
	uint8_t sensor_type;
	uint8_t checksum;
	uint8_t crc;
} teros_data;

teros_return teros_init(
		teros **teros,
		uart_inst_t *serial,
		teros_model model,
		int uart_tx_pin,
		int uart_rx_pin,
		int pwr_pin,
		teros_substrate substrate_type);
teros_return teros_get_data(teros *teros, teros_data *data);
teros_return teros_get_checksum(teros *teros, uint8_t *checksum);
teros_return teros_get_crc(teros *teros, uint8_t *crc);
teros_return teros_get_sensor_type(teros *teros, uint8_t *type);
teros_return teros_get_vwc(teros *teros, float *vwc);
teros_return teros_get_vwc_raw(teros *teros, float *vwc);
teros_return teros_get_permittivity(teros *teros, float *permittivity);
teros_return teros_get_tempc(teros *teros, float *temp);

float raw_to_m3m3_mineral(float raw);
float raw_to_m3m3_soilless(float raw);

static float text_to_float(char *text);

#endif 
