#include "rudp.h"
#include "pico/rand.h"
#include "string.h"

RUDP_RETURN rfm69_rudp_transmit(
        Rfm69 *rfm, 
        uint8_t address,
        uint8_t *payload, 
        uint payload_size,
        uint timeout,
        uint8_t retries
)
{
	// This count does not include the RBT packet
	uint8_t num_packets = payload_size/PAYLOAD_MAX;
    if (payload_size % PAYLOAD_MAX) num_packets++;
    // This payload is too large and should be fplit into multiple transmissions
    if (num_packets > TX_PACKETS_MAX) return RUDP_PAYLOAD_OVERFLOW; 

    // Cache previous op mode so it can be restored
    // after transmit.
    uint8_t previous_mode;
    rfm69_mode_get(rfm, &previous_mode);

    // Ensures registers are set correctly
    _rudp_init(rfm);

	uint8_t seq_num = get_rand_32() % SEQ_NUM_RAND_LIMIT;

    uint8_t size_bytes[sizeof(payload_size)];
    for (int i = 0; i < sizeof(payload_size); i++)
        size_bytes[i] = (payload_size >> (((sizeof(payload_size) - 1) * 8) - (i * 8))) & 0xFF;

    // Get our tx_address;
    uint8_t tx_address;
    rfm69_node_address_get(rfm, &tx_address);

	// Build header
	uint8_t header[HEADER_SIZE];
	header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE + sizeof(payload_size);
	header[HEADER_RX_ADDRESS]  = address;
	header[HEADER_TX_ADDRESS]  = tx_address;
	header[HEADER_FLAGS]       = HEADER_FLAG_RBT;
	header[HEADER_SEQ_NUMBER]  = seq_num;

    // Buffer for receiving ACK/RACK
    // Max possible size for ACK/RACK packets
    uint8_t ack_packet[HEADER_SIZE + num_packets];

    bool rbt_success = false;
    // Dirt goes here
    RUDP_RETURN rval = RUDP_TIMEOUT;
    for (uint retry = 0; retry <= retries; retry++) {
        
        rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

        // Write header to fifo
        rfm69_write(
                rfm,
                RFM69_REG_FIFO,
                header,	
                HEADER_SIZE
        );

        // Write payload size as payload
        rfm69_write(
                rfm,
                RFM69_REG_FIFO,
                size_bytes,	
                sizeof(payload_size)
        );

        
        if (retry) printf("Retrying RBT\n");
        else printf("Sending RBT\n");
        
        rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
        _rudp_block_until_packet_sent(rfm);
        printf("RBT sent to (%u): %u\n", address, seq_num);

        // I emply "backoff" where the timout increases with each retry plus "jitter"
        // This allows you to have a quick retry followed by successively slower retries
        // with some random deviation to avoid a certain class of timing bugs
        uint next_timeout = timeout + (retry * timeout) + (get_rand_32() % 100);
        // Retry if ACK was not received within timeout
        if (_rudp_rx_ack(rfm, seq_num + 1, next_timeout) == RUDP_TIMEOUT) continue;

        // Ack received
        rbt_success = true;
        break;
    }
    if (!rbt_success) goto CLEANUP; // Do not pass go

    seq_num += 2; // Set to first data packet seq num

    uint8_t seq_num_max = seq_num + num_packets - 1;

    uint8_t size;
    uint8_t offset;
    for (int i = 0; i < num_packets; i++) {
        rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

        // Not graceful
        if (seq_num + i == seq_num_max && payload_size % PAYLOAD_MAX)
            size = payload_size % PAYLOAD_MAX;
        else
            size = PAYLOAD_MAX;

        header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE + size;
        header[HEADER_FLAGS]       = HEADER_FLAG_DATA;
        header[HEADER_SEQ_NUMBER]  = seq_num + i;

        uint offset = PAYLOAD_MAX * i;

        // Write header to fifo
        rfm69_write(
                rfm,
                RFM69_REG_FIFO,
                header,	
                HEADER_SIZE
        );

        // Write slice of payload
        rfm69_write(
                rfm,
                RFM69_REG_FIFO,
                &payload[offset],
                size
        );

        sleep_ms(TX_INTER_PACKET_DELAY);
        rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

        _rudp_block_until_packet_sent(rfm);
        printf("Sending data packet: %u\n", seq_num + i);
    }

    uint8_t message_size = num_packets;
    uint rack_timeout;
    uint8_t packet_num;
    uint8_t is_ok;
    for (;;) {
        printf("Waiting for RACK\n");


        retries = TX_REQ_RACK_RETRIES;
        is_ok = false;
        while (retries) {
            retries--;
            if (_rudp_rx_rack(rfm, seq_num_max, TX_RACK_TIMEOUT, ack_packet) == RUDP_TIMEOUT) {
                rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);
                
                header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE; 
                header[HEADER_FLAGS]       = HEADER_FLAG_DATA | HEADER_FLAG_RACK;
                header[HEADER_SEQ_NUMBER]  = seq_num;

                rfm69_write(
                        rfm,
                        RFM69_REG_FIFO,
                        header,
                        HEADER_SIZE
                );

                sleep_ms(TX_INTER_PACKET_DELAY);
                rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
                _rudp_block_until_packet_sent(rfm);
                printf("Rack timout: Requesting new RACK\n");
                continue;

            }
            is_ok = ack_packet[HEADER_FLAGS] & HEADER_FLAG_OK;
            break;
        }
        if (is_ok) break;
        
        message_size = ack_packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE; 
        for (int i = 0; i < message_size; i++) {
            rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

            packet_num = ack_packet[PAYLOAD_BEGIN + i]; 

            // Not graceful still
            if (packet_num == seq_num_max && payload_size % PAYLOAD_MAX)
                size = payload_size % PAYLOAD_MAX;
            else
                size = PAYLOAD_MAX;

            header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE + size;
            header[HEADER_SEQ_NUMBER]  = packet_num;

            uint offset = PAYLOAD_MAX * (packet_num - seq_num);

            // Write header to fifo
            rfm69_write(
                    rfm,
                    RFM69_REG_FIFO,
                    header,	
                    HEADER_SIZE
            );

            // Write slice of payload
            rfm69_write(
                    rfm,
                    RFM69_REG_FIFO,
                    &payload[offset],
                    size
            );

            sleep_ms(TX_INTER_PACKET_DELAY);
            rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

            _rudp_block_until_packet_sent(rfm);
            printf("Sending data packet: %u\n", packet_num);

        }
    }

    rval = RUDP_OK;
CLEANUP:
    rfm69_mode_set(rfm, previous_mode);
    return rval;
}

static void _rudp_init(Rfm69 *rfm) {
    // Always start in stdby
	rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);
	// Set payload length max to FIFO max
	// For variable length packets, this only matters when receiving
    rfm69_payload_length_set(rfm, RFM69_FIFO_SIZE);

	// Set packet format to variable
	rfm69_packet_format_set(rfm, RFM69_PACKET_VARIABLE);	
}

static RUDP_RETURN _rudp_rx_rack(
        Rfm69 *rfm,
        uint8_t seq_num,
        uint timeout,
        uint8_t *packet
)
{
    RUDP_RETURN rval = RUDP_TIMEOUT;
    uint8_t is_rack;
    bool is_seq;
    uint8_t is_ok;

    rfm69_mode_set(rfm, RFM69_OP_MODE_RX);

    absolute_time_t timeout_time = make_timeout_time_ms(timeout);
    for (;;) {
        if (get_absolute_time() > timeout_time) break;

        if (!_rudp_is_payload_ready(rfm)) {
            continue;
        }

        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                packet,
                HEADER_SIZE
        );

        uint8_t message_size = packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE; 
        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                &packet[PAYLOAD_BEGIN],
                message_size 
        );

        // This is a RACK packet, which is what we wanted
        is_rack = (packet[HEADER_FLAGS] & HEADER_FLAG_RACK);
        // is it the correct sequence num?
        is_seq = packet[HEADER_SEQ_NUMBER] == seq_num;
        printf("is_rack:\t%u\nis_seq:\t%u\nis_ok:\t%u\n", is_rack, is_seq, is_ok);
        printf("size:\t%u\n", packet[HEADER_PACKET_SIZE]);
        printf("rx:\t%u\n", packet[HEADER_RX_ADDRESS]);
        printf("tx:\t%u\n", packet[HEADER_TX_ADDRESS]);
        printf("flags:\t%u\n", packet[HEADER_FLAGS]);
        printf("seq #:\t%u\n", packet[HEADER_SEQ_NUMBER]);
        printf("expected:%u\n", seq_num);
        printf("first:%u\n", packet[PAYLOAD_BEGIN]);

        if (!is_rack || !is_seq) continue;
        // An ok flag says we are good to stop transmitting
        printf("Received RACK\n");

        // RACK RECEIVED
        rval = RUDP_OK; 
        break;
    }
    return rval;
}

static RUDP_RETURN _rudp_rx_ack(
        Rfm69 *rfm,
        uint8_t seq_num,
        uint timeout
)
{
    RUDP_RETURN rval = RUDP_TIMEOUT;
    uint8_t packet[HEADER_SIZE];
    bool state;
    bool is_ack;
    bool is_seq;

    rfm69_mode_set(rfm, RFM69_OP_MODE_RX);

    absolute_time_t timeout_time = make_timeout_time_ms(timeout);
    for (;;) {
        if (get_absolute_time() > timeout_time) break;

        if (!_rudp_is_payload_ready(rfm)) {
            continue;
        }
        // No need to check length byte, an ack packet is only a header
        // with some flags set
        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                packet,
                HEADER_SIZE
        );

        // This is an RBT/ACK packet, which is what we wanted
        is_ack = (packet[HEADER_FLAGS] & (HEADER_FLAG_ACK | HEADER_FLAG_RBT)) > 0;
        // is it the correct sequence num?
        is_seq = packet[HEADER_SEQ_NUMBER] == seq_num;
        if (!is_ack || !is_seq) continue;

        printf("Ack received: %u\n", packet[HEADER_SEQ_NUMBER]);

        // ACK RECEIVED
        rval = RUDP_OK; 
        break;
    }
    return rval;
}

static inline bool _rudp_is_payload_ready(Rfm69 *rfm) {
    bool state;
    rfm69_irq2_flag_state(rfm, RFM69_IRQ2_FLAG_PAYLOAD_READY, &state);
    return state;
}

static inline void _rudp_block_until_packet_sent(Rfm69 *rfm) {
    bool state = false;
    while (!state) {
        rfm69_irq2_flag_state(rfm, RFM69_IRQ2_FLAG_PACKET_SENT, &state);
        sleep_us(1);
    }
}


RUDP_RETURN rfm69_rudp_receive(
        Rfm69 *rfm, 
		uint8_t *address,
        uint8_t *payload, 
        uint *payload_size,
        uint timeout
)
{
    // Cache previous op mode so it can be restored
    // after RX
    uint8_t previous_mode;
    rfm69_mode_get(rfm, &previous_mode);

    uint payload_buffer_size = *payload_size;
    *payload_size = 0;

    // Ensures registers are set correctly
    _rudp_init(rfm);

    uint8_t rx_address;
    rfm69_node_address_get(rfm, &rx_address);

    // Max size packet buffer
    uint8_t packet[RFM69_FIFO_SIZE];
    // Header buffer
    uint8_t header[HEADER_SIZE];

    uint8_t is_rbt;
    uint8_t seq_num;
    RUDP_RETURN rval = RUDP_TIMEOUT;

    printf("Waiting for RBT\n");

    absolute_time_t timeout_time = make_timeout_time_ms(timeout);
    uint8_t tx_started;
RESTART_RBT_LOOP: // This is to return to the RBT loop in case of a false
                  // start receiving the transmission
    tx_started = false;
    for (;;) {
        if (get_absolute_time() >= timeout_time) break;

        memset(packet, 0, RFM69_FIFO_SIZE);

        rfm69_mode_set(rfm, RFM69_OP_MODE_RX);

        if (!_rudp_is_payload_ready(rfm)) {
            sleep_us(1);
            continue;
        }

        rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                packet,
                HEADER_SIZE
        );

        is_rbt = packet[HEADER_FLAGS] & HEADER_FLAG_RBT;

        if (!is_rbt) {
            // Empty the FIFO
            rfm69_read(
                    rfm,
                    RFM69_REG_FIFO,
                    packet,
                    packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE
            );
            continue;
        } 

        // Read expected payload size
        uint8_t size_bytes[sizeof(*payload_size)];
        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                size_bytes,
                sizeof(*payload_size) 
        );

        for (int i = 0; i < sizeof(*payload_size); i++) 
            *payload_size |= size_bytes[i] << (((sizeof(payload_size) - 1) * 8) - (i * 8));

        // Get the sender's node address
        *address = packet[HEADER_TX_ADDRESS];
        // Increment the sequence
        seq_num = packet[HEADER_SEQ_NUMBER] + 1;
        printf("RBT received from (%u): %u\n", *address, seq_num - 1);
        printf("Expecting payload_size: %u\n", *payload_size);

        // Build ACK packet header
        header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE;
        header[HEADER_RX_ADDRESS]  = *address;
        header[HEADER_TX_ADDRESS]  = rx_address;
        header[HEADER_FLAGS]       = HEADER_FLAG_RBT | HEADER_FLAG_ACK;
        header[HEADER_SEQ_NUMBER]  = seq_num;

        rfm69_write( 
                rfm,
                RFM69_REG_FIFO,
                header,	
                HEADER_SIZE
        );

        rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
        _rudp_block_until_packet_sent(rfm);
        printf("ACK sent to (%u): %u\n", *address, seq_num);
        tx_started = true;
        break;
    }
    // If we have broken from the loop but tx hasn't started,
    // we have timed out
    if (!tx_started) goto CLEANUP;


	uint8_t num_packets_expected = *payload_size/PAYLOAD_MAX;
    if (*payload_size % PAYLOAD_MAX) num_packets_expected++;

    // We have our first data packet waiting in the FIFO now
    // Set our data packet seq num bounds
    uint8_t seq_num_max = seq_num + num_packets_expected;
    seq_num++;

    bool packets_received[TX_PACKETS_MAX] = {false}; // Keep track of what packets we have received
    uint8_t num_packets_missing = num_packets_expected;

    uint payload_bytes_received = 0; 

    uint8_t is_data;
    uint8_t packet_num;
    uint8_t is_req_rack;

    absolute_time_t rack_timeout = make_timeout_time_ms(RX_DATA_TIMEOUT * num_packets_missing + RX_DATA_LOOP_TIME);
    absolute_time_t now;
    while (num_packets_missing) {
        now = get_absolute_time();
        if (now >= timeout_time) goto CLEANUP;

        if (now >= rack_timeout) {
            printf("Rack timeout: Sending Rack\n");
            rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);
            // Time to send a RACK
            uint8_t size = (num_packets_missing > PAYLOAD_MAX) ? PAYLOAD_MAX : num_packets_missing;

            header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE + size;
            header[HEADER_FLAGS] = HEADER_FLAG_RACK;
            header[HEADER_SEQ_NUMBER] = seq_num_max;

            printf("flags: %u\n", header[HEADER_FLAGS]);
            printf("seq #: %u\n", header[HEADER_SEQ_NUMBER]);

            bool state;
            rfm69_irq2_flag_state(rfm, RFM69_IRQ2_FLAG_FIFO_NOT_EMPTY, &state);
            printf("fne: %u\n", state);

            //sleep_ms(15);
            
            rfm69_write( 
                    rfm,
                    RFM69_REG_FIFO,
                    header,	
                    HEADER_SIZE
            );

            // We are actually limited by packet size how many
            // missing packets we can report. Hopefully we aren't losing
            // 61+ packets in a single TX, but worst case scenario is
            // we have to send another RACK later
            uint8_t missing_packet;
            printf("Missing packets:\n");
            for (int i = 0; size; i++) {
                if (packets_received[i]) continue;
                missing_packet = i + seq_num;
                rfm69_write( 
                        rfm,
                        RFM69_REG_FIFO,
                        &missing_packet,	
                        1
                );
                size--;
                printf("%u\n", missing_packet);
            }

            rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

            rack_timeout = make_timeout_time_ms(RX_DATA_TIMEOUT * num_packets_missing + RX_DATA_LOOP_TIME);
            _rudp_block_until_packet_sent(rfm);
        }

        // Make sure packet is sent before leaving TX
        rfm69_mode_set(rfm, RFM69_OP_MODE_RX);
        
        if (!_rudp_is_payload_ready(rfm)) {
            sleep_us(1);
            continue;
        }

        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                packet,
                HEADER_SIZE
        );

        uint message_size = packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE;
        rfm69_read(
            rfm,
            RFM69_REG_FIFO,
            &packet[PAYLOAD_BEGIN],
            message_size
        );


        if (*address != packet[HEADER_TX_ADDRESS]) continue;

        is_rbt = packet[HEADER_FLAGS] & HEADER_FLAG_RBT;
        if (is_rbt) goto RESTART_RBT_LOOP;

        is_data = packet[HEADER_FLAGS] & HEADER_FLAG_DATA;
        if (!is_data) continue;

        packet_num = packet[HEADER_SEQ_NUMBER];
        if (packet_num < seq_num || packet_num > seq_num_max) continue;

        // Check if this is a request RAck
        is_req_rack = packet[HEADER_FLAGS] & HEADER_FLAG_RACK;
        if (is_req_rack && packet_num == seq_num) {
            printf("Received RACK REQ\n");
            rack_timeout = 0;
            continue;
        }

        printf("Received packet: %u\n", packet_num);

        // Account for packet only if it is a new packet
        if (packets_received[packet_num - seq_num]) continue;
        packets_received[packet_num - seq_num] = true;

        num_packets_missing--;

        payload_bytes_received += message_size;
        printf("Payload B: %u\n", payload_bytes_received);
        if (payload_bytes_received > payload_buffer_size) {
            rval = RUDP_RX_BUFFER_OVERFLOW;
            goto CLEANUP;
        }

        // Copy the payload data into the payload buffer
        uint payload_offset = PAYLOAD_MAX * (packet_num - seq_num);
        for (int i = 0; i < message_size; i++) {
            payload[payload_offset + i] = packet[PAYLOAD_BEGIN + i];    
        }
    }

    rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);
    header[HEADER_PACKET_SIZE] = HEADER_EFFECTIVE_SIZE;
    header[HEADER_FLAGS] = HEADER_FLAG_RACK | HEADER_FLAG_OK;
    header[HEADER_SEQ_NUMBER] = seq_num_max;

    // Send a non-guaranteed success packet
    printf("Sending OK\n");
    rfm69_write(
            rfm,
            RFM69_REG_FIFO,
            header,
            HEADER_SIZE
    );

    printf("Success:\n-RX BYTES:\t%u\n-NUM PACKETS:\t%u\n-PAYLOAD:", payload_bytes_received, num_packets_expected);
    //for (int i = 0; i < *payload_size; i++) {
    //    if (i % 8 == 0) printf("\n\t");
    //    printf("0x%02X ", payload[i]);
    //}
    printf("\n\n");

    sleep_ms(15);
    rfm69_mode_set(rfm, RFM69_OP_MODE_TX);
    _rudp_block_until_packet_sent(rfm);

    rval = RUDP_OK;
CLEANUP:
    rfm69_mode_set(rfm, previous_mode);
    return rval;
}
