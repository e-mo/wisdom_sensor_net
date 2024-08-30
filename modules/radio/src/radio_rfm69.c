#include <string.h>

#include "radio_interface.h"
#include "rfm69_rp2040.h"

static rfm69_context_t _rfm;
static rudp_context_t _rudp;
static bool _radio_init = false;

bool radio_init(void) {
	_radio_init  = false;

	// SPI init
    spi_init(RFM69_SPI, 1000*1000);
    gpio_set_function(RFM69_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(RFM69_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(RFM69_PIN_MOSI, GPIO_FUNC_SPI);

	// Drive CS pin high
    gpio_init(RFM69_PIN_CS);
    gpio_set_dir(RFM69_PIN_CS, GPIO_OUT);
    gpio_put(RFM69_PIN_CS, 1);

	struct rfm69_config_s config = {
		.spi      = RFM69_SPI,
		.pin_cs   = RFM69_PIN_CS,
		.pin_rst  = RFM69_PIN_RST
	};

	if (rfm69_init(&_rfm, &config) == false) {
		radio_error_set(RADIO_HW_FAILURE);
		goto RETURN;
	}

	if (rfm69_rudp_init(&_rudp, &_rfm) == false) {
		radio_error_set(RADIO_HW_FAILURE);
		goto RETURN;
	}

	_radio_init = true;
	radio_error_set(RADIO_OK);
RETURN:

	return _radio_init;
}

bool radio_address_set(uint8_t address) {
	if (_radio_init == false) {
		radio_error_set(RADIO_UNINITIALIZED);
		return false;
	}

	if (!rfm69_rudp_address_set(&_rudp, address)) {
		radio_error_set(RADIO_HW_FAILURE);
		return false;
	}

	return true;
}

RADIO_ERROR_T radio_status(char dst[ERROR_STR_MAX]) {	

	strncpy(dst, radio_error_str(), ERROR_STR_MAX);
	return radio_error_get();

}

// TODO: make this viable for any payload size
bool radio_send(void *payload, uint size, uint8_t address) {
	if (_radio_init == false) {
		radio_error_set(RADIO_UNINITIALIZED);
		return false;
	}

	if (!rfm69_rudp_payload_set(&_rudp, payload, size)) {
		radio_error_set(RADIO_HW_FAILURE);
		return false;
	}

	struct trx_report_s *report = rfm69_rudp_report_get(&_rudp);
	if (!rfm69_rudp_transmit(&_rudp, address)) {
		
		// Error depends on return status
		switch (report->return_status) {
		case RUDP_TIMEOUT:
			radio_error_set(RADIO_TX_TIMEOUT);
			break;
		default:
			radio_error_set(RADIO_TX_FAILURE);
		}

		return false;
	}

	if (report->return_status == RUDP_OK_UNCONFIRMED) {
		// TODO: Add data logging if transmissions was unconfirmed
	}

	return true;
}

bool radio_recv(void *buffer, uint size, uint *received) {
	if (_radio_init == false) {
		radio_error_set(RADIO_UNINITIALIZED);
		return false;
	}
	
	if (!rfm69_rudp_rx_buffer_set(&_rudp, buffer, size)) {
		radio_error_set(RADIO_HW_FAILURE);
		return false;
	}

	struct trx_report_s *report = rfm69_rudp_report_get(&_rudp);
	if (!rfm69_rudp_receive(&_rudp)) {
		
		// Error depends on return status
		switch (report->return_status) {
		case RUDP_TIMEOUT:
			radio_error_set(RADIO_RX_TIMEOUT);
			break;
		default:
			radio_error_set(RADIO_RX_FAILURE);
		}

		return false;
	}

	*received = report->bytes_received;

	return true;
}
