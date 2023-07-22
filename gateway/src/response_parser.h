#ifndef WISDOM_RESPONSE_PARSER_H
#define WISDOM_RESPONSE_PARSER_H

typedef struct _response_parser ResponseParser;

ResponseParser *rp_create();
void rp_destroy(ResponseParser *rp);

void rp_clear(ResponseParser *rp);

uint32_t rp_num_messages(ResponseParser *rp);

void rp_parse(ResponseParser *rp, uint8_t *src, uint32_t src_len);

static void rp_close_response(ResponseParser *rp, uint8_t len);

bool rp_contains(
		ResponseParser *rp, 
		uint8_t *needle,
		uint32_t n,
		uint8_t *index
); 

bool rp_contains_ok(ResponseParser *rp);

bool rp_get(ResponseParser *rp, uint8_t index, uint8_t **dst, uint8_t *dst_len);

bool rp_next(ResponseParser *rp, uint8_t **next, uint8_t *next_len);

#endif // WISDOM_RESPONSE_PARSER_H
