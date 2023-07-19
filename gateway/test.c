#include <stdio.h>
#include "command_buffer.h"

int main() {

	CommandBuffer *cb = command_buffer_create();
	if (!cb) fprintf(stderr, "cb malloc failed\n");

	printf("cb empty?: %s\n", 
			command_buffer_empty(cb) ? "true" : "false" );
	printf("prefix set?: %s\n", 
			command_buffer_prefix_set(cb) ? "true" : "false" );
	printf("prefix set again?: %s\n", 
			command_buffer_prefix_set(cb) ? "true" : "false" );
	uint32_t len = command_buffer_length(cb);
	printf("len: %u\n", len);

}
