// error_report.h
// Functions for reporting and responding to errors.
// NOT THREAD SAFE. If I wind up using both cores (which I suspect I will)
// I have left myself a note to come back and make this library safe for
// asynchronous code.
//
// Based on the classic errno.h library.
#ifndef ERROR_REPORT_H
#define ERROR_REPORT_H

enum ERR_CODE {
    ERR_NONE,
    ERR_INIT_MALLOC,
    ERR_INIT_TEST,
};

// Sets last_error. Must be called before any error reading or reporting
// functions.
void set_last_error(enum ERR_CODE err_code);

// Returns last error set with set_last_error
// for when you might want to know what the error was,
// but don't wish to report it or halt the process.
enum ERR_CODE get_last_error();

// Call if a critical non-recoverable error has occurred.
// Halts the main process (endless loop).
//
// Prints to USB UART if ERROR_UART is defined.
// Prints to log (not yet implemented) if ERROR_LOG is defined.
// Blink error code on Pico LED if ERROR_LED is defined.
// Number of blinks determined by the int value of the ERR_CODE (ex. INIT_MALLOC = 1)
void critical_error();

// Call if a recoverable error has occurred, but you still want
// to log/print the error. Does not halt main process.
void report_error();

#endif // ERROR_REPORT_H