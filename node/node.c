#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#include "rfm69.h"
#include "error_report.h"

#define ever ;; 

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20
#define PIN_IRQ_0  21
#define PIN_IRQ_1  21

void set_bi() {
    bi_decl(bi_program_name("Leaf Node"));
    bi_decl(bi_program_description("WISDOM sensor network node communications routine."))
    bi_decl(bi_1pin_with_name(PIN_MISO, "MISO"));
    bi_decl(bi_1pin_with_name(PIN_CS, "CS"));
    bi_decl(bi_1pin_with_name(PIN_SCK, "SCK"));
    bi_decl(bi_1pin_with_name(PIN_MOSI, "MOSI"));
    bi_decl(bi_1pin_with_name(PIN_RST, "RST"));
    bi_decl(bi_1pin_with_name(PIN_IRQ_0, "IRQ 0"));
    bi_decl(bi_1pin_with_name(PIN_IRQ_1, "IRQ 0"));
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf

    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode, which we want

    Rfm69 *rfm;
    uint rval = rfm69_init(
        &rfm,
        SPI_PORT,
        PIN_MISO,
        PIN_MOSI,
        PIN_CS,
        PIN_SCK,
        PIN_RST,
        PIN_IRQ_0,
        PIN_IRQ_1
    );
    
    rfm69_reset(rfm);
    rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

    rfm69_bitrate_set(rfm, RFM69_MODEM_BITRATE_250);
    rfm69_frequency_set(rfm, 915);

    rfm69_mode_set(rfm, RFM69_OP_MODE_SLEEP);

    // Check if rfm69_init was successful (== 0)
    // Set last error and halt process if not.
    if (rval != 0) {
        set_last_error(rval); // Can use return value from rfm69_init directly
        critical_error();
    }


    for(ever) { 
        uint32_t frequency;
        rfm69_frequency_get(rfm, &frequency);
        // if this prints ~915Mhz, everthing is working
        printf("freq: %u\n", frequency);
        sleep_ms(1000);
        uint16_t bitrate;
        rfm69_bitrate_get(rfm, &bitrate);
        printf("bitr: 0x%04X\n", bitrate);
        sleep_ms(1000);
    }
    
    return 0;
}


