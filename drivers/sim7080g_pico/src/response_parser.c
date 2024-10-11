#include <stdlib.h>
#include <stdbool.h>

#include "response_parser.h"

#define RP_RESPONSE_LEN_MAX 2000

ResponseParser *rp_create() {
	ResponseParser *rp = malloc((sizeof *rp));
	if (!rp) return NULL;

	rp->num_messages = 0;
	rp->index = 0;
}

void rp_destroy(ResponseParser *rp) {
	free(rp);
}

ResponseParser *rp_reset(ResponseParser *rp) {
	rp->num_messages = 0;
	rp->index = 0;
	return rp;
}

uint32_t rp_num_messages(ResponseParser *rp) {
	return rp->num_messages;	
}

void rp_parse(ResponseParser *rp, uint8_t *src, uint32_t src_len) {
	bool prefix = false;
	bool non_standard_response = false;
	bool close_response = false;
	uint8_t response_len = 0;


	for (int i = 0; i < src_len; i++) {
		if (src[i] == '\r') {
			continue;
		}

		if (src[i] == '\n') {
			
			if (non_standard_response) {
				non_standard_response = false;
				rp_close_response(rp, response_len);		
				response_len = 0;
			}

			if (!prefix) {
				prefix = true;
				continue;
			}


			rp_close_response(rp, response_len);
			prefix = false;
			response_len = 0;
			continue;
		}

		if (!prefix) {
			non_standard_response = true;
		}
		rp->buffer[rp->num_messages][response_len] = src[i];
		response_len++;
		if (response_len > RP_RESPONSE_LEN_MAX) break;
	}

	// If we are at the end of the src but have an unclosed
	// response
	if (non_standard_response || prefix)
		rp_close_response(rp, response_len);		
}

static void rp_close_response(ResponseParser *rp, uint8_t len) {
	rp->length_array[rp->num_messages] = len;
	rp->num_messages++;
}

bool rp_contains(
		ResponseParser *rp, 
		uint8_t *needle,
		uint32_t n,
		uint8_t *index
) 
{
	bool found = false;

	if (index == NULL) index = &(uint8_t) {0};

	for (*index = 0; *index < rp->num_messages; (*index)++) {
		if (n > rp->length_array[*index]) continue;

		for (int i = 0; i < n; i++) {
			if (needle[i] != rp->buffer[*index][i]) {
				found = false;
				continue;
			}
			found = true;
		}

		if (found) break;
	}

	return found;
}

bool rp_contains_ok(ResponseParser *rp) {
	return rp_contains(rp, "OK", 2, NULL);
}

bool rp_contains_err(ResponseParser *rp) {
	return rp_contains(rp, "+CME ERROR", 10, NULL);
}

bool rp_contains_ok_or_err(ResponseParser rp[static 1]) {
	return rp_contains_ok(rp) || rp_contains_err(rp);
}

bool rp_get(ResponseParser *rp, uint8_t index, uint8_t **dst, uint32_t *dst_len) {
	if (index >= rp->num_messages) return false;

	*dst = &rp->buffer[index][0];
	*dst_len = rp->length_array[index];
}

bool rp_next(ResponseParser *rp, uint8_t **next, uint8_t *next_len) {
	if (rp->index == rp->num_messages) return false;

	*next = &rp->buffer[rp->index][0];
	*next_len = rp->length_array[rp->index++];

	return true;
}
