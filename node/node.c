#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "rfm69hcw.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20
#define PIN_IRQ  21

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

// Reset procedure per RFM69HCW datasheet
void rfm_reset(void) {
    gpio_put(PIN_RST, 1);
    sleep_ms(100);
    gpio_put(PIN_RST, 0);
    sleep_ms(5);
}

void rfm_write(uint8_t address, uint8_t *buf, uint16_t len) {

}

void rfm_read(uint8_t address, uint8_t *buf, uint16_t len) {

}

void init(void) {
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Per documentation we leave RST pin floating for at least
    // 10 ms on startup. No harm in waiting 10ms here to
    // guarantee.
    sleep_ms(10); 
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 0);
}

int main()
{
    init();
    uint8_t buf[4];
    buf[0] = 0x04;
    rfm69hcw_write(SPI_PORT, PIN_CS, REG_OP_MODE, buf, 1);
    buf[0] = 0x00;
    //printf("%d", rval);
    while (1) {
        int rval = rfm69hcw_read(SPI_PORT, PIN_CS, REG_OP_MODE, buf, 4);
        printf("%d\n", rval);
        for (int i = 0; i < 4; i++) {
            printf("0x%02X\n", buf[i]);
        }
        sleep_ms(1000);
    }
    
    return 0;
}