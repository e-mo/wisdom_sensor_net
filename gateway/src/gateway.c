#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "hardware/uart.h"
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
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  21
#define PIN_IRQ_0  21
#define PIN_IRQ_1  21

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
					  
	Modem *modem = modem_start(
			MODEM_APN,
			UART_PORT,
			UART_PIN_TX,
			UART_PIN_RX,
			MODEM_PIN_PWR
	);
	
	// Start modem core
	//while (!tud_cdc_connected()) { sleep_ms(100); };
	multicore_launch_core1(modem_core_main);

	Rfm69 *rfm = rfm69_create();
	Rfm69Config config = {
		.spi      = SPI_PORT,
	    .pin_miso = PIN_MISO,
	 	.pin_cs   = PIN_CS,
	 	.pin_sck  = PIN_SCK,
		.pin_mosi = PIN_MOSI,
		.pin_rst  = PIN_RST,
		.pin_irq0 = 21,
		.pin_irq1 = 21
	};

	rfm69_rudp_init(rfm, &config);

	common_radio_config(rfm);
	rfm69_node_address_set(rfm, 0x00);

    TrxReport report;

	if (modem_cn_activate(modem, true)) printf("Network activated\n");

	if (modem_ssl_enable(modem, false)) printf("SSL disabled\n");

	if (modem_tcp_open(modem, strlen(SERVER_URL), SERVER_URL, SERVER_PORT)) {
		printf("TCP connection opened\n");
	}

	for(;;) {

        uint8_t address;
        uint size = 100000;
        uint8_t payload[size];

        printf("Receiving...\n");

        bool success = rfm69_rudp_receive(
                rfm,
                &report,
                &address,
                payload,
                &size,
                12000,
                2000
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

		printf("%u\n", modem_tcp_send(modem, 15, payload));

		sleep_ms(1000);

	}

    
    return 0;
}
