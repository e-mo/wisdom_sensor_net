#include <stdio.h>
#include "gw_core_error.h"

static GW_CORE_ERROR_T _gw_core_error = GW_CORE_UNINITIALIZED;

static const char *_error_str_lookup[GW_CORE_ERROR_MAX] = {
	[GW_CORE_OK] = "gw_core: ok",
	[GW_CORE_UNINITIALIZED] = "gw_core: uninitialized",
	[GW_CORE_FAILURE] = "gw_core: core system failure"
};

static const char *_error_format_str = "%s(%u): %s";
static char _error_str[ERROR_STR_MAX] = "";

bool _gw_core_error_set_verbose(GW_CORE_ERROR_T error, uint line, char *file) {
	if (error < 0 || error >= GW_CORE_ERROR_MAX) return false;

	_gw_core_error = error;

	snprintf(_error_str, ERROR_STR_MAX, _error_format_str, file, line, _error_str_lookup[_gw_core_error]);

	return true;
}

GW_CORE_ERROR_T gw_core_error_get(void) {
	return _gw_core_error;
}

const char * gw_core_error_str(void) {
	return _error_str;
}
