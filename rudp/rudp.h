#ifndef RFM69_RUDP_H
#define RFM69_RUDP_H

#include "rfm69.h"

typedef enum _RUDP_RETURN {
    RUDP_OK,
    RUDP_OK_UNCONFIRMED,
    RUDP_TIMEOUT,
    RUDP_BUFFER_OVERFLOW,
    RUDP_PAYLOAD_OVERFLOW
} RUDP_RETURN;

typedef struct _tx_report {
    uint payload_size;
    uint packets_sent;
    uint rbt_retries;
    uint retransmissions; 
    uint racks_received;
    uint rack_requests;
    RUDP_RETURN return_status;
    uint8_t tx_address;
    uint8_t rx_address;
    uint8_t num_packets;
} tx_report_t;

typedef struct _rx_report {
    uint bytes_expected;
    uint bytes_received;
    uint packets_received;
    uint acks_sent;
    uint racks_sent;
    uint rack_requests;
    RUDP_RETURN return_status;
    uint8_t rx_address;
    uint8_t tx_address;
} rx_report_t;

// I might eliminate this. I don't think it is really needed.
#define TX_INTER_PACKET_DELAY 0 

// The constant amount of time it takes to receive one data

#define _RX_DATA_TIMEOUT 12000
#define RX_DATA_TIMEOUT (TX_INTER_PACKET_DELAY + _RX_DATA_TIMEOUT)
#define TX_RACK_TIMEOUT 300

enum HEADER {
    HEADER_PACKET_SIZE,
    HEADER_RX_ADDRESS,
    HEADER_TX_ADDRESS,
    HEADER_FLAGS,
    HEADER_SEQ_NUMBER,
    HEADER_SIZE // Keep this at end
};
#define PAYLOAD_BEGIN (HEADER_SIZE)

#define HEADER_EFFECTIVE_SIZE (HEADER_SIZE - 1) // HEADER_SIZE - length byte (it isn't part of its own count)
#define PAYLOAD_MAX (65 - HEADER_EFFECTIVE_SIZE)
#define SEQ_NUM_RAND_LIMIT 25 
// 256 (byte packet num max) - potential range for starting seq num - 1 ack packet
#define TX_PACKETS_MAX (256 - SEQ_NUM_RAND_LIMIT - 1) 

enum FLAG {
    HEADER_FLAG_RBT  = 0x80,
    HEADER_FLAG_DATA = 0x40,
    HEADER_FLAG_ACK  = 0x20,
    HEADER_FLAG_RACK = 0x10,
    HEADER_FLAG_OK   = 0x08,
};


// rfm     - pointer to Rfm69 struct
// address - receiver node address
// payload - data payload to be sent
// length  - length of data payload in bytes
bool rfm69_rudp_transmit(
        Rfm69 *rfm, 
        tx_report_t *report,
        uint8_t address,
        uint8_t *payload, 
        uint payload_size, 
        uint timeout,
        uint8_t retries
);

static void _rudp_init(Rfm69 *rfm);

static RUDP_RETURN _rudp_rx_ack(
        Rfm69 *rfm,
        uint8_t seq_num,
        uint timeout
);

static RUDP_RETURN _rudp_rx_rack(
        Rfm69 *rfm,
        uint8_t seq_num,
        uint timeout,
        uint8_t *header
);

static inline bool _rudp_is_payload_ready(Rfm69 *rfm);

static inline void _rudp_block_until_packet_sent(Rfm69 *rfm);

// rfm     - pointer to Rfm69 struct
// address - returns with TX address
// payload - void buffer to recieve payload
// length  - should be passed containing length of payload buffer (to prevent overflow)
//           and returns containing number of bytes actually received
bool rfm69_rudp_receive(
        Rfm69 *rfm, 
        rx_report_t *report,
		uint8_t *address,
        uint8_t *payload, 
        uint *payload_size,
        uint per_packet_timeout,
        uint timeout
);

#endif // RFM60_RUDP_H
