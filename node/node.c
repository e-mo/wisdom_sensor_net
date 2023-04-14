#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#include "rfm69.h"

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
#define PIN_IRQ  21

void init_rfm(Rfm69 *rfm) {
    // REG_OP_MODE
    // Set into sleep mode
    uint8_t buf[9];
    buf[0] = 0x00;

    // REG_DATA_MODUL
    // Set Packet mode
    // Set FSK
    // No modulation shaping
    buf[1] = 0x00;

    // REG_BITRATE*
    // Set bit rate to 250kb/s
    buf[2] = 0x00; // MSB
    buf[3] = 0x80; // LSB

    // REG_FDEV*
    // FDA + BRF/2 =< 500 kHz
    // Fdev(13,0) = 0x1000 = 4092 khz
    // FDEV = Fdev(13,0) * FSTEP (61Hz) = ~250,000kHz
    // 0.5 <= 2 * FDEV/BR <= 10 (MI)
    // 2 * FDEV/BR (250,000kb/s) = ~2
    // Knowing basically nothing about radio communications before a
    // month ago, I feel good about a 2 modulation index.
    buf[4] = 0x10;
    buf[5] = 0x00;

    // RRG_FRF*
    // Set the frequency to ~915MHz
    // Frf = Fstep * Frf(23,0)
    buf[6] = 0xE4;   
    buf[7] = 0xC0;
    buf[8] = 0x00;

    // Burst write 9 sequential registers starting with SPI_PORT
    rfm69_write(rfm, REG_OP_MODE, buf, 9);

    // REG_TEST_DAGC 
    // Fading margin improvement for AfcLowBetaOn = 0
    buf[0] = 0x30;
    rfm69_write(rfm, REG_OP_MODE, buf, 1);
}

int main()
{
    bi_decl(bi_program_name("Leaf Node"));
    bi_decl(bi_program_description("WISDOM sensor network leaf node communications routine."))
    bi_decl(bi_1pin_with_name(16, "MISO"));
    bi_decl(bi_1pin_with_name(17, "CS"));
    bi_decl(bi_1pin_with_name(18, "SCK"));
    bi_decl(bi_1pin_with_name(19, "MOSI"));
    bi_decl(bi_1pin_with_name(20, "RST"));
    bi_decl(bi_1pin_with_name(21, "IRQ 1"));
    bi_decl(bi_1pin_with_name(22, "IRQ 2"));

    stdio_init_all(); // To be able to use printf
    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode
    Rfm69 *rfm = rfm69_init(
        SPI_PORT,
        PIN_MISO,
        PIN_MOSI,
        PIN_CS,
        PIN_SCK,
        PIN_RST,
        PIN_IRQ
    );

    for(ever) { // hehe

    }
    
    return 0;
}