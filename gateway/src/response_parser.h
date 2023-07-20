#ifndef WISDOM_RESPONSE_PARSER_H
#define WISDOM_RESPONSE_PARSER_H

typedef struct _response_parser ResponseParser;

ResponseParser *rp_create();

void rp_destroy(ResponseParser *rp);

uint32_t rp_num_messages(ResponseParser *rp);

uint8_t rp_parse(ResponseParser *rp, uint8_t *src, uint32_t src_len);

#endif // WISDOM_RESPONSE_PARSER_H
