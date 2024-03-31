#include <stdio.h>
#include "gateway_error.h"

static GATEWAY_ERROR_T _gateway_error = GATEWAY_UNINITIALIZED;

static const char *_error_str_lookup[GATEWAY_ERROR_MAX] = {
	[GATEWAY_OK] = "gateway: ok",
	[GATEWAY_UNINITIALIZED] = "gateway: module uninitialized",
	[GATEWAY_MALLOC_FAILURE] = "gateway: malloc failure",
	[GATEWAY_HW_FAILURE] = "gateway: hardware failure",
	[GATEWAY_CORE_ERROR] = "gateway: core1 error"
};

static const char *_error_format_str = "%s(%u): %s";
static char _error_str[ERROR_STR_MAX] = "";

bool _gateway_error_set_verbose(GATEWAY_ERROR_T error, uint line, char *file) {
	if (error < 0 || error >= GATEWAY_ERROR_MAX) return false;

	_gateway_error = error;

	snprintf(_error_str, ERROR_STR_MAX, _error_format_str, file, line, _error_str_lookup[_gateway_error]);

	return true;
}

GATEWAY_ERROR_T gateway_error_get(void) {
	return _gateway_error;
}

const char * gateway_error_str(void) {
	return _error_str;
}
