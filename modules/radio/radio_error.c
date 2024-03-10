#include "radio_error.h"

static RADIO_ERROR_T _radio_error = RADIO_ERROR_UNINITIALIZED;

static char *_error_str_lookup[RADIO_ERROR_MAX] = {
	[RADIO_ERROR_OK] = "ok",
	[RADIO_ERROR_UNINITIALIZED] = "module uninitialized"
};

char * radio_error_str(void) {
	return _error_str_lookup[_radio_error];
}
