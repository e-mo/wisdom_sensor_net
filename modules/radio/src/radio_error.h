#ifndef WISDOM_RADIO_ERROR_H
#define WISDOM_RADIO_ERROR_H

#include <stdbool.h>

#define ERROR_STR_MAX (100)

typedef unsigned uint;

typedef enum _radio_error {
	RADIO_OK,
	RADIO_UNINITIALIZED,
	RADIO_MALLOC_FAILURE,
	RADIO_HW_FAILURE,
	RADIO_TX_TIMEOUT,
	RADIO_TX_FAILURE,
	RADIO_RX_TIMEOUT,
	RADIO_RX_FAILURE,
	RADIO_ERROR_MAX
} RADIO_ERROR_T;

#define radio_error_set(e) _radio_error_set_verbose(e, __LINE__, __FILE__)
bool _radio_error_set_verbose(RADIO_ERROR_T error, uint line, char *file);

RADIO_ERROR_T radio_error_get(void);
const char * radio_error_str(void);

#endif // WISDOM_RADIO_ERROR_H
