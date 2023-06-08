#ifndef RFM69_RUDP_H
#define RFM69_RUDP_H

#include "rfm69.h"

typedef enum _RUDP_RETURN {
} RUDP_RETURN;

// rfm     - pointer to Rfm69 struct
// address - receiver node address
// payload - data payload to be sent
// length  - length of data payload in bytes
//
// If payload length > 253 bytes (255 - 1(address byte) - 1(length byte)),
// payload will be sent in multiple packets. 
RUDP_RETURN rfm69_rudp_transmit(
        Rfm69 *rfm, 
        uint8_t address,
        void *payload, 
        uint length
);

// rfm     - pointer to Rfm69 struct
// payload - void buffer to recieve payload
// length  - should be passed containing length of void buffer (to prevent overflow)
//           and returns containing number of bytes actually received
RUDP_RETURN rfm69_rudp_receive(
        Rfm69 *rfm, 
        void *payload, 
        uint *length
);

#endif // RFM60_RUDP_H
