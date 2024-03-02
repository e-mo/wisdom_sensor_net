// rudp_init.c
// Example of Rfm69 RUDP initialization
// Also great for testing if your radio is connected properly
//
// Prints success=true to serial every second if init was successful
// Prints success=false otherwise

#include <stdio.h>

#include "pico/stdlib.h"

#include "tusb.h"
#include "rfm69_pico.h"

int main() {
    stdio_init_all(); // To be able to use printf

	bool success = false;
	char *error = "success";

	// Create rfm69 context
	rfm69_context_t *rfm = rfm69_create();
	if (rfm == NULL) {
		error = "rfm69_create returned NULL";
		goto LOOP_BEGIN;
	}

	// Pull in definitions from winsdom_config.cmake
	// spi0 is a macro defined in Pico API
	rfm69_config_t config = {
		.spi      = spi0,
		.pin_miso = RFM69_PIN_MISO,
		.pin_cs   = RFM69_PIN_CS,
		.pin_sck  = RFM69_PIN_SCK,
		.pin_mosi = RFM69_PIN_MOSI,
		.pin_rst  = RFM69_PIN_RST
	};
	// Initialize rfm69 context
	if (rfm69_init(rfm, &config) == false) {
		error = "rfm69_init failed";
		goto LOOP_BEGIN;
	}
	
	// Create rudp context
	rudp_context_t *rudp = rfm69_rudp_create();
	if (rudp == NULL) {
		error = "rfm69_rudp_create returned NULL";
		goto LOOP_BEGIN;
	}
	// Initialize rudp context
	//if (rfm69_rudp_init(rudp, rfm) == false) {
	//		error = "rfm69_rudp_init failed";
//		goto LOOP_BEGIN;
//	}

	rfm69_reset(rfm);

	rfm69_mode_set(rfm, RFM69_OP_MODE_SLEEP);
	rfm69_data_mode_set(rfm, RFM69_DATA_MODE_PACKET);
	rfm69_node_address_set(rfm, 0x02);
	rfm69_bitrate_set(rfm, RFM69_MODEM_BITRATE_2_4);
	rfm69_fdev_set(rfm, 2400);
	rfm69_frequency_set(rfm, 915);
	rfm69_rxbw_set(rfm, RFM69_RXBW_MANTISSA_24, 6);
	rfm69_dcfree_set(rfm, RFM69_DCFREE_WHITENING);
	rfm69_power_level_set(rfm, 10);
	rfm69_packet_format_set(rfm, RFM69_PACKET_FIXED);

	rfm69_rssi_threshold_set(rfm, 0xE4);
	rfm69_tx_start_condition_set(rfm, RFM69_TX_FIFO_NOT_EMPTY);
	rfm69_broadcast_address_set(rfm, 0xFF); 
	rfm69_address_filter_set(rfm, RFM69_FILTER_NODE_BROADCAST);

	rfm69_dagc_set(rfm, RFM69_DAGC_IMPROVED_0);

	//Set sync value (essentially functions as subnet)
	uint8_t sync[3] = {0x01, 0x01, 0x01};
	rfm69_sync_value_set(rfm, sync, 3);

	rfm69_payload_length_set(rfm, 2);

	rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

	success = true;	

LOOP_BEGIN:; 

	rfm69_mode_set(rfm, RFM69_OP_MODE_RX); 
	for(;;) {


		bool state = false;
		while (!state) {
			rfm69_irq2_flag_state(rfm, RFM69_IRQ2_FLAG_PAYLOAD_READY, &state);
		}

		printf("payload ready\n");
		rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

		uint8_t buffer[2];
		rfm69_read(
				rfm,
				RFM69_REG_FIFO,
				buffer,
				2
		);

		printf("data: %02X\n", buffer[0]);
		printf("data: %02X\n", buffer[1]);

		//rfm69_mode_set(rfm, RFM69_OP_MODE_RX); 
	}
    
    return 0;
}
