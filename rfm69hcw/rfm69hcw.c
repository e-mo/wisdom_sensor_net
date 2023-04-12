#include "rfm69hcw.h"

void rfm69hcw_reset(uint pin_rst) {
    gpio_put(pin_rst, 1);
    sleep_ms(100);
    gpio_put(pin_rst, 0);
    sleep_ms(5);
}

// I actually have no idea why this is necessary, but every
// single SPI example in the documentation spends three cycles
// doing nothing before and after the cs pin is set/cleared.
// I'm not going to question it.
static inline void cs_select(uint pin_cs) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin_cs, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint pin_cs) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin_cs, 1);
    asm volatile("nop \n nop \n nop");
}


int rfm69hcw_write(spi_inst_t* spi, 
                    const uint pin_cs,
                    uint8_t address, 
                    const uint8_t *src,
                    size_t len)
{
    address |= 0x80; // Set rw bit
    cs_select(pin_cs);
    spi_write_blocking(spi, &address, 1);
    int rval = spi_write_blocking(spi, src, len);
    cs_deselect(pin_cs);
    return rval;
}

int rfm69hcw_read(spi_inst_t* spi, 
                   const uint pin_cs,
                   uint8_t address, 
                   uint8_t *dst,
                   size_t len)
{
    address &= 0x7F; // Clear rw bit
    cs_select(pin_cs);
    spi_write_blocking(spi, &address, 1);
    int rval = spi_read_blocking(spi, 0, dst, len);
    cs_deselect(pin_cs);
    return rval;
}