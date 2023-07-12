#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "hardware/spi.h"

#include "rfm69.h"
#include "rudp.h"
#include "error_report.h"
#include "teros_11.h"

#define BAUD
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

#define UART_TX 0
#define UART_RX 1
#define TEROS_PWR 3

void set_bi() {
    bi_decl(bi_program_name("Transmission Timing Calculator Tool"));
    bi_decl(bi_program_description("For finding per packet delay timings at certain baud rates"))
    bi_decl(bi_1pin_with_name(PIN_MISO, "MISO"));
    bi_decl(bi_1pin_with_name(PIN_CS, "CS"));
    bi_decl(bi_1pin_with_name(PIN_SCK, "SCK"));
    bi_decl(bi_1pin_with_name(PIN_MOSI, "MOSI"));
    bi_decl(bi_1pin_with_name(PIN_RST, "RST"));
    //bi_decl(bi_1pin_with_name(PIN_IRQ_0, "IRQ 0"));
    //bi_decl(bi_1pin_with_name(PIN_IRQ_1, "IRQ 1"));
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf

	// TEROS INIT
	teros *teros;
	teros_data t11;

	sleep_ms(2000);
	teros_init( //initialize it!
    		&teros,
			uart0, //pick a uart
		    teros_11, //teros_11 or teros_12
	    	UART_TX,
		    UART_RX,
	    	TEROS_PWR,
		    mineral //soilless or mineral
	);
	sleep_ms(3000);

    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode, which we want

    Rfm69 *rfm = rfm69_create();
    rfm69_rudp_init(
        rfm,
        SPI_PORT,
        PIN_MISO,
        PIN_MOSI,
        PIN_CS,
        PIN_SCK,
        PIN_RST,
        PIN_IRQ_0,
        PIN_IRQ_1
    );

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    // 250kb/s baud rate
    rfm69_bitrate_set(rfm, RFM69_MODEM_BITRATE_57_6);
    // ~2 beta 
    rfm69_fdev_set(rfm, 70000);
    // 915MHz 
    rfm69_frequency_set(rfm, 915);
    //rfm69_modulation_shaping_set(rfm, RFM69_FSK_GAUSSIAN_0_3);
    // RXBW >= fdev + br/2
    rfm69_rxbw_set(rfm, RFM69_RXBW_MANTISSA_20, 2);
    //rfm69_power_level_set(rfm, -2);


	sleep_ms(3000);
	uint num_packets = 10;
	uint8_t buf[66] = {65, 0xff};
    for(ever) { 

		rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);
		for (int i = 0; i < num_packets; i++) {
			rfm69_write(
					rfm,
					RFM69_REG_FIFO,
					buf,
					1
			);
			rfm69_write(
					rfm,
					RFM69_REG_FIFO,
					&buf[1],
					65	
			);

			printf("here?\n");
			rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
			bool state = false;
			while (!state) {
				rfm69_irq2_flag_state(rfm, RFM69_IRQ2_FLAG_PACKET_SENT, &state);
				printf("here\n");
				sleep_us(1);
			}
		}

		printf("sent\n");
		sleep_ms(1000);
	}

    return 0;
}


