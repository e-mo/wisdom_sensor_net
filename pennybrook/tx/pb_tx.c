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
    bi_decl(bi_program_name("Test Transmitter"));
    bi_decl(bi_program_description("WISDOM sensor network tx test."))
    bi_decl(bi_1pin_with_name(PIN_MISO, "MISO"));
    bi_decl(bi_1pin_with_name(PIN_CS, "CS"));
    bi_decl(bi_1pin_with_name(PIN_SCK, "SCK"));
    bi_decl(bi_1pin_with_name(PIN_MOSI, "MOSI"));
    bi_decl(bi_1pin_with_name(PIN_RST, "RST"));
    //bi_decl(bi_1pin_with_name(PIN_IRQ_0, "IRQ 0"));
    //bi_decl(bi_1pin_with_name(PIN_IRQ_1, "IRQ 1"));
}

void print_registers(Rfm69 *rfm) {
    uint8_t test = 0x00;
    for (int i = 1; i < 0x50; i++) {
        rfm69_read(
                rfm,
                i,
                &test,
                1
        );
        printf("0x%02X: 0x%02X\n", i, test);
    }
    rfm69_read(
            rfm,
            0x5A,
            &test,
            1
    );
    printf("0x5A: 0x%02X\n", test);
    rfm69_read(
            rfm,
            0x5C,
            &test,
            1
    );
    printf("0x5C: 0x%02X\n", test);
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf

	// TEROS INIT
	teros *teros;
	teros_data t11;

	teros_init( //initialize it!
    		&teros,
			uart0, //pick a uart
		    teros_11, //teros_11 or teros_12
	    	UART_TX,
		    UART_RX,
	    	TEROS_PWR,
		    mineral //soilless or mineral
	);

    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode, which we want

    Rfm69 *rfm = rfm69_create();
    bool success = rfm69_rudp_init(
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
    rfm69_dcfree_set(rfm, RFM69_DCFREE_WHITENING);

    rfm69_node_address_set(rfm, 0x01); 

    //rfm69_power_level_set(rfm, 2);
	TrxReport report;
	uint buf_size = sizeof(float) * 2;
	float buf[2] = {0.0};

    //print_registers(rfm);
    sleep_ms(2000);
    for(ever) { 
		// Zero our buffer memset(buf, 0x00, buf_size);

		//if (teros_get_data(teros, &t11)) {
		//	printf("Teros Error!\n");
		//	sleep_ms(1000);
		//	continue;
		//}

		//buf[0] = t11.vwc;
		//buf[1] = t11.temperature;
		buf[0] = 0;
		buf[1] = 0;

        printf("Transmitting...\n");

        success = rfm69_rudp_transmit(
                rfm,
                &report,
                0x02,
                (uint8_t *)buf,
                buf_size,
                300,
                5 
        );

        printf("Report\n");
        printf("------\n");
        printf("     tx_address: %u\n", report.tx_address);
        printf("     rx_address: %u\n", report.rx_address);
        printf("   payload_size: %u\n", report.payload_size);
        printf("   packets_sent: %u\n", report.data_packets_sent);
        printf("retransmissions: %u\n", report.data_packets_retransmitted);
        printf(" racks_received: %u\n", report.racks_received);
        printf("  rack_requests: %u\n", report.rack_requests_sent);
        switch(report.return_status) {
            case RUDP_OK:
                printf("  return_status: RUDP_OK\n");
                break;
            case RUDP_OK_UNCONFIRMED:
                printf("  return_status: RUDP_OK_UNCONFIRMED\n");
                break;
            case RUDP_TIMEOUT:
                printf("  return_status: RUDP_TIMEOUT\n");
                break;
            case RUDP_PAYLOAD_OVERFLOW:
                printf("  return_status: RUDP_PAYLOAD_OVERFLOW\n");
                break;
        }
        printf("\n");

        if (success) {
            uint num_blinks = 3;
            for(; num_blinks; num_blinks--) {
                gpio_put(PICO_DEFAULT_LED_PIN, 1);
                sleep_ms(100);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
                sleep_ms(50);
            }
        } else {
            sleep_ms(1000);
            continue;
        }

        sleep_ms(1000);
    }
    
    return 0;
}


