// C library headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "rp2x_serial.h"

typedef unsigned uint;


int main(int argc, char **argv) {

	int serial_port = rp2x_serial_open("/dev/ttyACM0", 10, 0, 115200);
	if (serial_port == -1) return 1;

	char command[100] = {0};

	int rval;
	char *cp = command;
	size_t length = 0;
	for (;;) {
			
		*cp++ = getc(stdin);
		if (*cp == '\n')
			printf("fart");

		//mp = message;
		//remaining = 5;
		//while (remaining) {
		//	rval = write(serial_port, mp, remaining);
		//	if (rval <= 0) return 2;
		//	mp += rval;
		//	remaining -= rval;
		//}
	}

	return 0;
};
