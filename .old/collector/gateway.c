#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "hardware/uart.h"
#include "hardware/adc.h"
#include "tusb.h"

#include "modem.h"
#include "command_buffer.h"
#include "rfm69.h"
#include "rudp.h"
#include "sensors.h"
#include "common_config.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

#define BUF_SIZE 4096

// 12-bit conversion, assume max value == ADC_VREF == 3.3 V

const float ADC_CONVERSION_FACTOR = 3.3f / (1 <<12);

void set_bi() {
    bi_decl(bi_program_name("Wisdom Gateway"));
    bi_decl(bi_program_description("Wisdom gateway software"))
}

void send_bytes_to_modem(size_t num_bytes, char bytes[num_bytes]) {

	uint32_t next = 0;
	for (int i = 0, j = 0; i < num_bytes; i++, j++) {
		if (j == 4) {
			multicore_fifo_push_blocking(next);
			next = 0;
			j = 0;
		}

		next |= bytes[i] << (8 * (3 - j));
	}

	if (next) multicore_fifo_push_blocking(next);
}

int main() {
    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf
	
	// Start modem core
	while (!tud_cdc_connected()) { sleep_ms(100); };

	adc_init();
	adc_gpio_init(26);
	adc_gpio_init(27);
	adc_gpio_init(28);

	//printf("here\n");

	//float test = 43.1545299;
	//uint8_t bytes[100];
	//bytes[0] = TEROS_11;
	//bytes[1] = sprintf(&bytes[2], "%2.9f", test);
	//printf("%u\n", bytes[1]);
	//printf("%.*s\n", bytes[1], &bytes[2]);
	//send_bytes_to_modem(bytes[1] + 2, bytes);

	Rfm69 *rfm = rfm69_create();
	Rfm69Config config = {
		.spi      = spi0,
		.pin_miso = 16,
		.pin_cs   = 17,
		.pin_sck  = 18,
		.pin_mosi = 19,
	    .pin_rst  = 20,
		.pin_irq0 = 21,
		.pin_irq1 = 21
	};

	rfm69_rudp_init(rfm, &config);
	common_radio_config(rfm);
    rfm69_node_address_set(rfm, 0x01); 
	
	uint16_t wet = 2500;
	uint16_t dry = 3440;
	uint16_t range = dry - wet;
	TrxReport report;
	bool success;

	for(ever) {

		uint8_t data[sizeof (float) * 3 + 3];

		for (int i = 0; i < 3; i++) {
			adc_select_input(i);
			switch (i) {
				case 0:
					printf("\nblack\n---\n");
					break;
				case 1:
					printf("\nwhite\n---\n");
					break;
				case 2:
					printf("\ngreen\n---\n");
					break;
			}
			int16_t result = adc_read();
			printf("range: %u\n", range);
			printf("result before: %u\n", result);
			result -= wet;
			if (result > range) result = range;
			if (result < 0) result = 0;
			printf("result after: %u\n", result);

			float moisture_percentage = 100.0f - (((float) result / (float) range) * 100);		

			printf("\%: %02f\n", moisture_percentage);

			data[i * 5] = i;
			memcpy(&data[i * 5 + 1], &moisture_percentage, sizeof (float));

		}

        printf("Transmitting...\n");

        success = rfm69_rudp_transmit(
                rfm,
                &report,
                0x00,
                (uint8_t *)data,
                15,
                100,
                10
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

		sleep_ms(1000);

	}
    
    return 0;
}
