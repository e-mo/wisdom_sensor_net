#ifndef RFM69_RUDP_H
#define RFM69_RUDP_H

#include "rfm69.h"

typedef enum _RUDP_RETURN {
    RUDP_OK,
    RUDP_TIMEOUT,
    RUDP_RX_BUFFER_OVERFLOW,
    RUDP_PAYLOAD_OVERFLOW
} RUDP_RETURN;

#define TX_RTP_TIMEOUT 1000 // 100ms ack timout
#define TX_RTP_RETRIES 5
#define TX_REQ_RACK_RETRIES 5

#define TX_INTER_PACKET_DELAY 0

#define RX_DATA_LOOP_TIME 110
#define _RX_DATA_TIMEOUT 11 
#define RX_DATA_TIMEOUT (TX_INTER_PACKET_DELAY + _RX_DATA_TIMEOUT)
#define TX_RACK_TIMEOUT 1000

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
RUDP_RETURN rfm69_rudp_transmit(
        Rfm69 *rfm, 
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
RUDP_RETURN rfm69_rudp_receive(
        Rfm69 *rfm, 
		uint8_t *address,
        uint8_t *payload, 
        uint *payload_size,
        uint timeout
);

#endif // RFM60_RUDP_H
