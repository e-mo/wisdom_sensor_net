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
	ok			=  0;
	uart_not_available	= -1;


#endif 
