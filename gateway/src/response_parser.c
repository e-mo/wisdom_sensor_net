#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "response_parser.h"

#define RESPONSE_NUM_MAX 100
#define RESPONSE_LEN_MAX 100

struct _response_parser {
	uint8_t buffer[RESPONSE_NUM_MAX][RESPONSE_LEN_MAX];
	uint32_t num_messages;
	uint8_t *index;
	uint32_t length_array[100];
};

ResponseParser *rp_create() {
	ResponseParser *rp = malloc((sizeof *rp));
	if (!rp) return NULL;

	rp->num_messages = 0;
	rp->index = &rp->buffer[0][0];
}

void rp_destroy(ResponseParser *rp) {
	free(rp);
}
uint32_t rp_num_messages(ResponseParser *rp) {
	return rp->num_messages;	
}

uint8_t rp_parse(ResponseParser *rp, uint8_t *src, uint32_t src_len) {
	for (int i = 0; i < src_len; i++) {
		if (src[i] == '\r') printf("\\r");
		else if (src[i] == '\n') printf("\\n\n");
		else printf("%c", src[i]);
	}
	return 0;
}
