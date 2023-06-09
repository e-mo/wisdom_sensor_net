#include "rudp.h"

#define HEADER_SIZE 5
#define PAYLOAD_SIZE 66

RUDP_RETURN rfm69_rudp_transmit(
        Rfm69 *rfm, 
        uint8_t address,
        void *payload, 
        uint length
)
{
	uint8_t packet[PAYLOAD_SIZE + HEADER_SIZE];
	uint8_t num_packets = length/PAYLOAD_SIZE;
	

	rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

	// Set payload length max to FIFO max
    rfm69_payload_length_set(rfm, 66);

	// Set packet format to variable
	rfm69_packet_format_set(rfm, RFM69_PACKET_VARIABLE);	

	// Fill the Fifo	
		
	
	// enter TX
	rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
	// Ignoring any large packet operating for now
}

RUDP_RETURN rfm69_rudp_receive(
        Rfm69 *rfm, 
        void *payload, 
        uint *length
)
{
	rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

	// Set payload length max to FIFO max
    rfm69_payload_length_set(rfm, 66);
	rfm69_packet_format_set(rfm, RFM69_PACKET_VARIABLE);	

}
