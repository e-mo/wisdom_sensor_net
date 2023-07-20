#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/rand.h"
#include "hardware/spi.h"

#include "rfm69.h"
#include "rudp.h"
#include "common_config.h"

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
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf

    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode, which we want

    Rfm69 *rfm = rfm69_create();
    uint rval = rfm69_rudp_init(
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

	common_radio_config(rfm);

    rfm69_node_address_set(rfm, 0x86); 
    rfm69_power_level_set(rfm, 0);

    bool success;
    TrxReport report;
    for(ever) { 
		printf("Waiting for payload to relay\n");
		printf("...\n");

        uint8_t address;
        uint size = 100000;
        uint8_t payload[size];

        success = rfm69_rudp_receive(
                rfm,
                &report,
                &address,
                payload,
                &size,
                13000,
                30000
        );

        printf("Report\n");
        printf("------\n");
        printf("      tx_address: %u\n", report.tx_address);
        printf("      rx_address: %u\n", report.rx_address);
        printf("  bytes_expected: %u\n", report.payload_size);
        printf("  bytes_received: %u\n", report.bytes_received);
        printf("packets_received: %u\n", report.data_packets_received);
        printf("       acks_sent: %u\n", report.acks_sent);
        printf("      racks_sent: %u\n", report.racks_sent);
        printf("   rack_requests: %u\n", report.rack_requests_received);

        switch(report.return_status) {
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
        printf("         payload: %s\n", payload);
        printf("\n");

        if (!success) continue;

        printf("relaying payload: %s\n", payload);
        printf("...\n");

		// If we didn't recieve anything, there is nothing to send.

		uint num_blinks = 3;
		for(; num_blinks; num_blinks--) {
			gpio_put(PICO_DEFAULT_LED_PIN, 1);
			sleep_ms(100);
			gpio_put(PICO_DEFAULT_LED_PIN, 0);
			sleep_ms(50);
		}

        success = rfm69_rudp_transmit(
                rfm,
                &report,
                0x02,
				payload,
                report.bytes_received,
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
        }

        sleep_ms(60000);
    }
    
    return 0;
}


