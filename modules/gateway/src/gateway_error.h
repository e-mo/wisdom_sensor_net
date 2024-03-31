#ifndef WISDOM_GATEWAY_ERROR_H
#define WISDOM_GATEWAY_ERROR_H

#include <stdbool.h>

typedef unsigned uint;

#define ERROR_STR_MAX (100)

typedef enum _gateway_error {
	GATEWAY_OK,		
	GATEWAY_UNINITIALIZED,
	GATEWAY_MALLOC_FAILURE,
	GATEWAY_HW_FAILURE,
	GATEWAY_CORE_ERROR,
	GATEWAY_ERROR_MAX
} GATEWAY_ERROR_T;

#define gateway_error_set(e) _gateway_error_set_verbose(e, __LINE__, __FILE__)
bool _gateway_error_set_verbose(GATEWAY_ERROR_T error, uint line, char *file);

GATEWAY_ERROR_T gateway_error_get(void);
const char * gateway_error_str(void);

#endif // WISDOM_GATEWAY_ERROR_H
