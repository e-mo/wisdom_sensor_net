#ifndef WISDOM_GW_CORE_ERROR_H
#define WISDOM_GW_CORE_ERROR_H

#include <stdbool.h>

#define ERROR_STR_MAX (100)

typedef unsigned uint;

typedef enum _gw_core_error {
	GW_CORE_OK,
	GW_CORE_UNINITIALIZED,
	GW_CORE_FAILURE,
	GW_CORE_ERROR_MAX
} GW_CORE_ERROR_T;

#define gw_core_error_set(e) _gw_core_error_set_verbose(e, __LINE__, __FILE__)
bool _gw_core_error_set_verbose(GW_CORE_ERROR_T error, uint line, char *file);

GW_CORE_ERROR_T gw_core_error_get(void);
const char * gw_core_error_str(void);

#endif // WISDOM_GW_CORE_ERROR_H
