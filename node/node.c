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

void init_rfm(Rfm69 *rfm) {
    rfm69_reset(rfm);
    // REG_OP_MODE
    // Set into sleep mode
    uint8_t buf[9];
    buf[0] = 0x00;

    // REG_DATA_MODUL
    // Set Packet mode
    // Set FSK
    // No modulation shaping
    buf[1] = 0x00;

    rfm69_write(rfm, RFM69_REG_OP_MODE, &buf[1], 1);
    // REG_BITRATE*
    // Set bit rate to 250kb/s

    // REG_FDEV*
    // FDA + BRF/2 =< 500 kHz
    // Fdev(13,0) = 0x1000 = 4092 khz
    // FDEV = Fdev(13,0) * FSTEP (61Hz) = ~250,000kHz
    // 0.5 <= 2 * FDEV/BR <= 10 (MI range)
    // 2 * FDEV/BR (250,000kb/s) = ~2 (MI)
    buf[4] = 0x10;
    buf[5] = 0x00;

    rfm69_bitrate_set(rfm, RFM69_MODEM_BITRATE_1_2);
    rfm69_frequency_set(rfm, 400);

    // Burst write 9 sequential registers starting with SPI_PORT

    // REG_TEST_DAGC 
    // Fading margin improvement for AfcLowBetaOn = 0
    buf[0] = 0x30;
    rfm69_write(rfm, RFM69_REG_TEST_DAGC, buf, 1);
}

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
    

    init_rfm(rfm);

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
    }
    
    return 0;
}


