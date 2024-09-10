#ifndef HIBERNATE_RP2040_H
#define HIBERNATE_RP2040_H

#include <stdbool.h>
typedef unsigned uint;

typedef enum {
    DORMANT_SOURCE_NONE,
    DORMANT_SOURCE_XOSC,
    DORMANT_SOURCE_ROSC
} dormant_source_t;

// Initialize the clocks after hibernation
void hibernate_init_clocks(void);

bool dormant_source_valid(dormant_source_t dormant_source);

void hibernate_run_from_dormant_source(dormant_source_t dormant_source);
void hibernate_goto_dormant_until_pin(uint gpio_pin, bool edge, bool high);

void hibernate_recover_clocks(uint clock0_orig, uint clock1_orig);

#endif // HIBERNATE_RP2040_H
