#include "rudp.h"
#include "pico/rand"

#define HEADER_SIZE  5  // Header is 5 bytes
#define PAYLOAD_MAX 61  // Max payload size in bytes

RUDP_RETURN rfm69_rudp_transmit(
        Rfm69 *rfm, 
        uint8_t address,
        void *payload, 
        uint size 
)
{

	rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);
	// Set payload length max to FIFO max
	// For variable length packets, this only matters when receiving
    rfm69_payload_length_set(rfm, 66);

	// Set packet format to variable
	rfm69_packet_format_set(rfm, RFM69_PACKET_VARIABLE);	

	// Packet sequence number starts in range from 0 -> 100
	// Does not limit number of packets because number 255
	// will loop around to 0.
	uint8_t rand_sequence_start = get_rand_32() % 100;

	// Build header
	uint8_t header[HEADER_SIZE];
	packet[0] = address;               // RX address
	packet[1] = HEADER_SIZE + 1;       // Size of first packet
	packet[2] = rfm->address;          // TX address
	packet[3] = RUDP_REQUEST_BEGIN_TX; // Info flags
	packet[4] = rand_sequence_start;        

	// Write header to fifo
	rfm69_write(
			rfm,
			RFM69_REG_FIFO,
			header,	
			HEADER_SIZE
	);

	// Write num_packets to fifo
	// This count does not include the RBT packet
	uint8_t num_packets = (size/PAYLOAD_MAX) + 1;
	rfm69_write(
			rfm,
			RFM69_REG_FIFO,
			&num_packets,	
			1
	);
	
	// enter TX and send RTP
	rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
	// Ignoring any large packet operating for now
}

RUDP_RETURN rfm69_rudp_receive(
        Rfm69 *rfm, 
		uint8_t *address,
        void *payload, 
        uint *length
)
{
	rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

	// Set payload length max to FIFO max
    rfm69_payload_length_set(rfm, 66);
	rfm69_packet_format_set(rfm, RFM69_PACKET_VARIABLE);	

}
