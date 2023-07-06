#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "hardware/spi.h"

#include "rfm69.h"
#include "rudp.h"
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
    bi_decl(bi_program_name("Test Repeater"));
    bi_decl(bi_program_description("WISDOM sensor network repeater test."))
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
    // Transmit starts with any data in the FIFO

    rfm69_node_address_set(rfm, 0x02); 

    // Check if rfm69_init was successful (== 0)
    // Set last error and halt process if not.
    if (rval != 0) {
        set_last_error(rval); // Can use return value from rfm69_init directly
        critical_error();
    }

    rfm69_power_level_set(rfm, 20);
    bool success;
    rx_report_t rx_report;
    tx_report_t tx_report;
    for(ever) { 

        uint8_t address;
        uint size = 100000;
        uint8_t payload[size];

        printf("Waiting for message\n");
        printf("...\n");

        success = rfm69_rudp_receive(
                rfm,
                &rx_report,
                &address,
                payload,
                &size,
                12000,
                30000
        );

        printf("Report\n");
        printf("------\n");
        printf("      tx_address: %u\n", rx_report.tx_address);
        printf("      rx_address: %u\n", rx_report.rx_address);
        printf("  bytes_expected: %u\n", rx_report.bytes_expected);
        printf("  bytes_received: %u\n", rx_report.bytes_received);
        printf("packets_received: %u\n", rx_report.packets_received);
        printf("       acks_sent: %u\n", rx_report.acks_sent);
        printf("      racks_sent: %u\n", rx_report.racks_sent);
        printf("   rack_requests: %u\n", rx_report.rack_requests);

        switch(rx_report.return_status) {
            case RUDP_OK:
                printf("   return_status: RUDP_OK\n");
                break;
            case RUDP_TIMEOUT:
                printf("   return_status: RUDP_TIMEOUT\n");
                break;
            case RUDP_BUFFER_OVERFLOW:
                printf("   return_status: RUDP_BUFFER_OVERFLOW\n");
                break;
        }
        printf("         message: %s\n", payload);
        printf("\n");

        char *message = "hi,campers";
        uint buf_size = strlen(message) + 1;
        //uint buf_size = get_rand_32() % 10000;
        //uint buf_size = TX_PACKETS_MAX * PAYLOAD_MAX;
        //uint buf_size = PAYLOAD_MAX;
        printf("Sending message: %s\n", message);
        printf("...\n");

		// If we didn't recieve anything, there is nothing to send.
        if (!success) continue;

		uint num_blinks = 3;
		for(; num_blinks; num_blinks--) {
			gpio_put(PICO_DEFAULT_LED_PIN, 1);
			sleep_ms(100);
			gpio_put(PICO_DEFAULT_LED_PIN, 0);
			sleep_ms(50);
		}

        success = rfm69_rudp_transmit(
                rfm,
                &tx_report,
                0x03,
                message,
                //buf,
                buf_size,
                300,
                5 
        );

        printf("Report\n");
        printf("------\n");
        printf("     tx_address: %u\n", tx_report.tx_address);
        printf("     rx_address: %u\n", tx_report.rx_address);
        printf("   payload_size: %u\n", tx_report.payload_size);
        printf("    num_packets: %u\n", tx_report.num_packets);
        printf("   packets_sent: %u\n", tx_report.packets_sent);
        printf("    rbt_retries: %u\n", tx_report.rbt_retries);
        printf("retransmissions: %u\n", tx_report.retransmissions);
        printf(" racks_received: %u\n", tx_report.racks_received);
        printf("  rack_requests: %u\n", tx_report.rack_requests);
        switch(tx_report.return_status) {
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
        }

        sleep_ms(1000);
    }
    
    return 0;
}


