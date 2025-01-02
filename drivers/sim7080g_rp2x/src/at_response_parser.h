#ifndef AT_RESPONSE_PARSER_H
#define AT_RESPONSE_PARSER_H

#include "at_definitions.h"

typedef enum at_response_types_e {
	AT_RESPONSE_OK,
	AT_RESPONSE_ERROR,
	AT_RESPONSE_CME_ERROR,
	AT_RESPONSE_UNSOLICITED
} AT_RESPONSE_TYPE;

struct at_response_info {
	AT_RESPONSE_TYPE type;	
	size_t length;
};

#endif // AT_RESPONSE_PARSER_H
