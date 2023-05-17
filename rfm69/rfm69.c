#include "rfm69.h"
#include "stdlib.h"
#include "math.h"
#include "pico/malloc.h"


struct Rfm69 {
    spi_inst_t* spi; // Initialized SPI instance
    uint pin_cs;
    uint pin_rst;
};

RFM69_ERR_CODE rfm69_init(
    Rfm69 **rfm,
    spi_inst_t *spi,
    uint pin_miso,
    uint pin_mosi,
    uint pin_cs,
    uint pin_sck,
    uint pin_rst,
    uint pin_irq_0,
    uint pin_irq_1
) 
{
    *rfm = malloc(sizeof(Rfm69));    
    if (rfm == NULL) return RFM69_INIT_MALLOC;

    (*rfm)->spi = spi;
    (*rfm)->pin_cs = pin_cs;
    (*rfm)->pin_rst;

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

    // Try to read version register
    uint8_t buf[1] = {0x00};
    rfm69_read(*rfm, RFM69_REG_VERSION, buf, 1);
    if (buf[0] == 0x00 || buf[0] == 0xFF) { return RFM69_INIT_TEST; }

    return RFM69_NO_ERROR;
}

void rfm69_reset(Rfm69 *rfm) {
    gpio_put(rfm->pin_rst, 1);
    sleep_us(100);
    gpio_put(rfm->pin_rst, 0);
    sleep_ms(15);
}

// I actually have no idea why this is necessary, but every
// single SPI example in the documentation spends three cycles
// doing nothing before and after the cs pin is set/cleared.
// Likely to allow time for pins to settle?
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
    address |= 0x80; // Clear rw bit
    cs_select(rfm->pin_cs);
    int rval = spi_write_blocking(rfm->spi, &address, 1);
    rval += spi_write_blocking(rfm->spi, src, len);
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
    int rval = spi_write_blocking(rfm->spi, &address, 1);
    spi_read_blocking(rfm->spi, 0, dst, len);
    cs_deselect(rfm->pin_cs);
    return rval;
}


int rfm69_irq1_flag_state(Rfm69 *rfm, RFM69_IRQ1_FLAG flag, bool *state) {
    uint8_t reg;
    int rval = rfm69_read(rfm, RFM69_REG_IRQ_FLAGS_1, &reg, 1);

    if (reg & flag) *state = true;
    else *state = false;

    return rval;
}

int rfm69_irq2_flag_state(Rfm69 *rfm, RFM69_IRQ2_FLAG flag, bool *state) {
    uint8_t reg;
    int rval = rfm69_read(rfm, RFM69_REG_IRQ_FLAGS_2, &reg, 1);

    if (reg & flag != 0) *state = true;
    else *state = false;

    return rval;
}

int rfm69_frequency_set(Rfm69 *rfm,
                        uint frequency)
{
    // Frf = Fstep * Frf(23,0)
    frequency *= 1000000; // MHz to Hz
    frequency = round(frequency / RFM69_FSTEP); // Gives needed register value
    // Split into three bytes.
    uint8_t buf[3] = {
        (frequency >> 16) & 0xFF,
        (frequency >> 8) & 0xFF,
        frequency & 0xFF
    };
    return rfm69_write(rfm, RFM69_REG_FRF_MSB, buf, 3);
}

int rfm69_frequency_get(Rfm69 *rfm, uint32_t *frequency) {
        uint8_t buf[3] = {0};
        int rval = rfm69_read(rfm, RFM69_REG_FRF_MSB, buf, 3);

        *frequency = (uint32_t) buf[0] << 16;
        *frequency |= (uint32_t) buf[1] << 8;
        *frequency |= (uint32_t) buf[2];
        *frequency *= RFM69_FSTEP;

        return rval;
}

int rfm69_bitrate_set(Rfm69 *rfm,
                      uint16_t bit_rate)
{
    uint8_t bytes[2] = {
        (bit_rate & 0xFF00) >> 8,
        bit_rate & 0xFF
    }; 
    return rfm69_write(rfm, RFM69_REG_BITRATE_MSB, bytes, 2);
}

int rfm69_bitrate_get(Rfm69 *rfm, uint16_t *bit_rate) {
    uint8_t buf[2] = {0}; 
    int rval = rfm69_read(rfm, RFM69_REG_BITRATE_MSB, buf, 2);

    *bit_rate = (uint16_t) buf[0] << 8;
    *bit_rate |= (uint16_t) buf[1];

    return rval;
}

int rfm69_mode_set(Rfm69 *rfm, RFM69_OP_MODE mode) {
    uint8_t reg;
    int rval = rfm69_read(rfm, RFM69_REG_OP_MODE, &reg, 1);

    reg &= ~(RFM69_OP_MODE_MASK); 
    reg |= mode & RFM69_OP_MODE_MASK;

    rval += rfm69_write(rfm, RFM69_REG_OP_MODE, &reg, 1);

    rval += rfm69_mode_wait_until_ready(rfm);

    return rval;
}

int rfm69_mode_ready(Rfm69 *rfm, bool *ready) {
    return rfm69_irq1_flag_state(rfm, RFM69_IRQ1_FLAG_MODE_READY, ready);
}

int rfm69_mode_wait_until_ready(Rfm69 *rfm) {
    int rval = 0;

    bool ready = false;
    while (!ready) {
        rval += rfm69_mode_ready(rfm, &ready);
    }

    return rval;
}
