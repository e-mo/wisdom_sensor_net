// C library headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <time.h>

#include "rtc_sync.h"

typedef unsigned uint;

int rp2x_serial_open(char *dev, int vtime, int vmin) {
	// Open the serial port. yACM0
	int serial_port = open(dev, O_RDWR);
	if (serial_port == -1)
		goto ERRNO_RETURN;

	// Create new termios struct, we call it 'tty' for convention
	struct termios tty;

	// Read in existing settings, and handle any error
	if(tcgetattr(serial_port, &tty) != 0)
		goto ERRNO_RETURN;

	tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
	tty.c_cflag |= CS8; // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	tty.c_cc[VTIME] = vtime;		// Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = vmin;

	// Set in/out baud rate to be 9600
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
		goto ERRNO_RETURN;

	return serial_port;
ERRNO_RETURN:
	if (serial_port != -1) close(serial_port);

	fprintf(stderr, "Failed to open device for serial communication: %s\n", dev);
	fprintf(stderr, "errno %i: %s\n", errno, strerror(errno));

	return -1;
}

int main(int argc, char **argv) {
	char *dev = NULL;
	if (argc > 1) 
		dev = argv[1];	
	else 
		dev = "/dev/ttyACM0"; // Default name (linux)
	
	printf("Running rtc_sync on device: %s\n", dev);

	uint buf = 0;
	int serial_port = rp2x_serial_open(dev, 10, sizeof buf);
	if (serial_port == -1) goto ERRNO;

	// Wait for a request
	ssize_t rval = read(serial_port, &buf, sizeof buf);

	if (rval == -1) goto ERRNO;
	if (rval == 0 || rval < sizeof buf) {
		printf("rval: %u\n", rval);
		goto TIMEOUT;
	}

	if (buf != RTC_SYNC_REQUEST) 
		goto BAD_REQUEST;

	// Send raw UTC time
	time_t rawtime;
	time(&rawtime);

	size_t len = sizeof rawtime;
	time_t *tp = &rawtime;
	while (len) {
		rval = write(serial_port, tp, len);
		if (rval <= 0) goto ERRNO;
		tp += rval;
		len -= rval;
	}

	// Check if sync success
	rval = read(serial_port, &buf, sizeof buf);

	if (rval == -1) goto ERRNO;
	if (rval == 0 || rval < sizeof buf) {
		printf("rval: %u\n", rval);
		goto TIMEOUT;
	}

	if (buf != RTC_SYNC_SUCCESS) 
		goto BAD_SUCCESS;
	
	close(serial_port);

	printf("RTC sync successful!\n");
	return 0; // success

ERRNO:
	fprintf(stderr, "errno %i: %s\n", errno, strerror(errno));
	goto RETURN_ERROR;

TIMEOUT:
	fprintf(stderr, "Serial communication timed out with device: %s\n", dev);
	goto RETURN_ERROR;

BAD_REQUEST:
	fprintf(stderr, "Bad RTC_SYNC_REQUEST byte sent\n");
	goto RETURN_ERROR;

BAD_SUCCESS:
	fprintf(stderr, "Bad RTC_SYNC_SUCCESS byte sent\n");

RETURN_ERROR:
	if (serial_port != -1) close(serial_port);
	fprintf(stderr, "Failed to sync RTC\n");
	return 1;
};
