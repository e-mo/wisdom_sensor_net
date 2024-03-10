#ifndef WISDOM_RADIO_ERROR_H
#define WISDOM_RADIO_ERROR_H

typedef enum _radio_error {
	RADIO_ERROR_OK,
	RADIO_ERROR_UNINITIALIZED,
	RADIO_ERROR_MAX
} RADIO_ERROR_T;

void radio_error_set(RADIO_ERROR_T error);

char * radio_error_str(void);

#endif // WISDOM_RADIO_ERROR_H
