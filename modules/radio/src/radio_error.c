#include <stdio.h>

#include "radio_error.h"

static RADIO_ERROR_T _radio_error = RADIO_UNINITIALIZED;

static const char *_error_str_lookup[RADIO_ERROR_MAX] = {
	[RADIO_OK] = "radio: ok",
	[RADIO_UNINITIALIZED] = "radio: module uninitialized",
	[RADIO_MALLOC_FAILURE] = "radio: malloc failure",
	[RADIO_TX_TIMEOUT] = "radio: tx timeout",
	[RADIO_TX_FAILURE] = "radio: tx failure",
	[RADIO_RX_TIMEOUT] = "radio: rx timeout",
	[RADIO_RX_FAILURE] = "radio: rx failure",
	[RADIO_HW_FAILURE] = "radio: hardware failure"
};

static const char *_error_format_str = "%s(%u): %s";
static char _error_str[ERROR_STR_MAX] = "";

bool _radio_error_set_verbose(RADIO_ERROR_T error, uint line, char *file) {
	if (error < 0 || error >= RADIO_ERROR_MAX) return false;

	_radio_error = error;

	snprintf(_error_str, ERROR_STR_MAX, _error_format_str, file, line, _error_str_lookup[_radio_error]);

	return true;
}

RADIO_ERROR_T radio_error_get(void) {
	return _radio_error;
}

const char * radio_error_str(void) {
	return _error_str;
}
