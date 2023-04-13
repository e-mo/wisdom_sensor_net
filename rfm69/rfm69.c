#include "rfm69.h"

struct Rfm69 {
    spi_inst_t* spi;
    uint pin_cs;
};

Rfm69 *rfm69_init(spi_inst_t *spi,
                  uint pin_miso,
                  uint pin_mosi,
                  uint pin_cs,
                  uint pin_sck,
                  uint pin_rst,
                  uint pin_irq)
{
    // malloc on a pico. Yeehaw.
    Rfm69 *rm = malloc(sizeof(Rfm69));    
    rm->spi = spi;
    rm->pin_cs = pin_cs;

    // SPI initialisation. This example will use SPI at 1MHz.
    gpio_set_function(pin_miso, GPIO_FUNC_SPI);
    gpio_set_function(pin_sck,  GPIO_FUNC_SPI);
    gpio_set_function(pin_mosi, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(pin_cs);
    gpio_set_dir(pin_cs, GPIO_OUT);
    gpio_put(pin_cs, 1);

    // Per documentation we leave RST pin floating for at least
    // 10 ms on startup. No harm in waiting 10ms here to
    // guarantee.
    sleep_ms(10); 
    gpio_init(pin_rst);
    gpio_set_dir(pin_rst, GPIO_OUT);
    gpio_put(pin_rst, 0);

    return rm;
}

void rfm69_reset(uint pin_rst) {
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


int rfm69_write(Rfm69 *rfm, 
                uint8_t address, 
                const uint8_t *src,
                size_t len)
{
    address |= 0x80; // Set rw bit
    cs_select(rfm->pin_cs);
    spi_write_blocking(rfm->spi, &address, 1);
    int rval = spi_write_blocking(rfm->spi, src, len);
    cs_deselect(rfm->pin_cs);
    return rval;
}

int rfm69_read(Rfm69 *rfm, 
               uint8_t address, 
               uint8_t *dst,
               size_t len)
{
    address &= 0x7F; // Clear rw bit
    cs_select(rfm->pin_cs);
    spi_write_blocking(rfm->spi, &address, 1);
    int rval = spi_read_blocking(rfm->spi, 0, dst, len);
    cs_deselect(rfm->pin_cs);
    return rval;
}