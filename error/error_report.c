#include "error_report.h"
#include <stdio.h>
#include "pico/stdlib.h"

enum ERR_CODE last_err = ERR_NONE;

static char *err_msg_table[] = {
    "no error reported",
    "rfm69_init: malloc failed to allocate memory",
    "rfm69_init: failed read version register test",
};

// TODO: Make this function thread safe
void set_last_error(enum ERR_CODE err_code) { 
    last_err = err_code;
}

enum ERR_CODE get_last_error() {
    return last_err;
}

void critical_error() {
#ifdef LOG_DEBUG
    // implement our logging procedure here
#endif // LOG_DEBUG

#ifdef ERROR_LED
    const uint led_pin = PICO_DEFAULT_LED_PIN;
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
#endif // ERROR_LED

    for(;;) {
        // Print associated error message from err_msg_table to UART
#ifdef ERROR_UART // Define in CMakeLists.txt to enable
        printf("Critical Error: %s\n", err_msg_table[last_err]);
#endif // ERROR_UART

#ifdef ERROR_LED
        for (int i = 0; i < last_err; i++) {
            gpio_put(led_pin, 1);
            sleep_ms(150);
            gpio_put(led_pin, 0);
            sleep_ms(150);
        }
#endif
        sleep_ms(3000); // If we display nothing, we still want to slow the loop.
                        // No reason to mash on the CPU and needlessly spend energy.
    }
}

void report_error() {

}