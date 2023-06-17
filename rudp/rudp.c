#include "rudp.h"
#include "pico/rand.h"

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
	uint8_t num_packets = (payload_size/PAYLOAD_MAX) + 1;
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
        
        rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

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

        if (seq_num + i == seq_num_max)
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

        sleep_ms(15);
        rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

        _rudp_block_until_packet_sent(rfm);
        printf("Sending data packet: %u\n", seq_num + i);
    }

    uint8_t message_size;
    uint8_t packet_num;
    for (;;) {
        printf("Waiting for RACK\n");
        if (_rudp_rx_rack(rfm, seq_num_max, TX_RACK_TIMEOUT, ack_packet) == RUDP_TIMEOUT) break;
        
        message_size = ack_packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE; 
        for (int i = 0; i < message_size; i++) {
            rfm69_mode_set(rfm, RFM69_OP_MODE_STDBY);

            packet_num = ack_packet[PAYLOAD_BEGIN + i]; 

            if (packet_num == seq_num_max)
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

            sleep_ms(15);
            rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

            _rudp_block_until_packet_sent(rfm);
            printf("Sending data packet: %u\n", seq_num + i);

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
    bool is_rack;
    bool is_seq;
    bool is_ok;

    rfm69_mode_set(rfm, RFM69_OP_MODE_RX);

    absolute_time_t timeout_time = make_timeout_time_ms(timeout);
    for (;;) {
        if (get_absolute_time() > timeout_time) break;

        if (!_rudp_is_payload_ready(rfm)) {
            sleep_us(1);
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

        // This is a RACK packet, which is what we wanted
        is_rack = (packet[HEADER_FLAGS] & HEADER_FLAG_RACK) > 0;
        // is it the correct sequence num?
        is_seq = packet[HEADER_SEQ_NUMBER] == seq_num;
        is_ok = (packet[HEADER_FLAGS] & HEADER_FLAG_OK) > 0;

        if (!is_rack || !is_seq) continue;
        // An ok flag says we are good to stop transmitting
        if (is_ok) {
            printf("Received OK RACK\n");
            break;
        }
        printf("Received RACK\n");

        uint8_t message_size = packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE; 

        rfm69_read(
                rfm,
                RFM69_REG_FIFO,
                &packet[PAYLOAD_BEGIN],
                message_size 
        );

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
            sleep_us(1);
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

    bool is_rbt;
    bool is_data;
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

        if (!is_rbt) continue;

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

        //sleep_ms(15);
        rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

        rfm69_write( 
                rfm,
                RFM69_REG_FIFO,
                header,	
                HEADER_SIZE
        );

        _rudp_block_until_packet_sent(rfm);
        printf("ACK sent to (%u): %u\n", *address, seq_num);
        tx_started = true;
        break;
    }
    // If we have broken from the loop but tx hasn't started,
    // we have timed out
    if (!tx_started) goto CLEANUP;


	uint8_t num_packets = (*payload_size/PAYLOAD_MAX) + 1;

    // We have our first data packet waiting in the FIFO now
    // Set our data packet seq num bounds
    uint8_t seq_num_max = seq_num + num_packets;

    seq_num++;
    uint8_t next_expected = seq_num;

    bool packets_received[TX_PACKETS_MAX] = {false}; // Keep track of what packets we have received

    uint8_t packets_missing[TX_PACKETS_MAX] = {0}; // Keep track of what packets we have received
    uint8_t num_packets_missing = 0;

    uint8_t num_packets_received = 0;
    uint payload_bytes_received = 0; 

    // First we read out the rest of the packet to make sure the FIFO is empty
    uint message_size = packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE;
    rfm69_read(
        rfm,
        RFM69_REG_FIFO,
        &packet[PAYLOAD_BEGIN],
        message_size
    );
    payload_bytes_received += message_size;

    if (payload_bytes_received > payload_buffer_size) {
        rval = RUDP_RX_BUFFER_OVERFLOW;
        goto CLEANUP;
    }

    // These are the two "oopsie" scenarios
    // If either of these don't match up, we have received a data packet
    // from an unexpected node, or the sequence num doesn't match what
    // we expected to receive. Either way, reenter the RBT timeout loop
    // because we left it early
    uint8_t tx_address = packet[HEADER_TX_ADDRESS];
    if (*address != tx_address) goto RESTART_RBT_LOOP;
    uint8_t packet_num = packet[HEADER_SEQ_NUMBER];
    if (packet_num < seq_num || packet_num > seq_num_max) goto RESTART_RBT_LOOP;

    // If this wasn't the next expected packet in sequence
    if (packet_num != next_expected) {
        for (int i = next_expected; i < packet_num; i++)
            packets_missing[num_packets_missing++] = i;
    }

    if (packet_num != seq_num_max) next_expected == packet_num + 1;

    printf("Received packet: %u\n", packet_num);
    // Mark received packet and increment count
    uint8_t packet_index = packet_num - seq_num;
    packets_received[packet_index] = true;
    num_packets_received++;

    // Copy the payload data into the payload buffer
    uint payload_offset = PAYLOAD_MAX * packet_index;
    for (int i = 0; i < message_size; i++) {
        payload[payload_offset + i] = packet[PAYLOAD_BEGIN + i];    
    }

    // We then continue to wait for data packets
    // If we hit the timeout_time before receiving all packets,
    // send RACK with missing packets, even if all (but initial) packets.
    //
    // If we receive all packets,
    // send a RACK/SUCCESS packet with no payload to end transmission.
    // If this gets lost, that is fine
    bool rack_sent = false;
    absolute_time_t rack_timeout = make_timeout_time_ms(RX_DATA_TIMEOUT);
    absolute_time_t now;
    while (num_packets_received < num_packets) {
        now = get_absolute_time();
        // If we are over the greater timeout time, the
        // transmission has taken too long and must time out

        if (now >= timeout_time) {
            printf("TIMEOUT!\n");
            goto CLEANUP;
        };

        if (now >= rack_timeout) {
            // Time to send a RACK
            uint8_t num_missing = num_packets - num_packets_received;
            uint8_t size = HEADER_EFFECTIVE_SIZE + num_missing;

            // The rest of the header should already be set properly
            // from sending the ACK
            header[HEADER_PACKET_SIZE] = size;
            header[HEADER_FLAGS] = HEADER_FLAG_RACK;
            header[HEADER_SEQ_NUMBER] = seq_num_max;


            //sleep_ms(15);
            rfm69_mode_set(rfm, RFM69_OP_MODE_TX);

            rfm69_write( 
                    rfm,
                    RFM69_REG_FIFO,
                    header,	
                    HEADER_SIZE
            );

            // This needs mega improvement
            uint8_t missing_packet;
            printf("Missing packets:\n");
            for (int i = 0; num_missing; i++) {
                if (packets_received[i]) continue;
                missing_packet = i + seq_num;
                rfm69_write( 
                        rfm,
                        RFM69_REG_FIFO,
                        &missing_packet,	
                        1
                );
                num_missing--;
                printf("\t%u\n", missing_packet);
            }

            _rudp_block_until_packet_sent(rfm);
            printf("RACK sent\n");
            rack_sent = true;

            rack_timeout = make_timeout_time_ms(RX_DATA_TIMEOUT * num_packets_missing);
            num_packets_missing = 0;
        }

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

        uint message_size = packet[HEADER_PACKET_SIZE] - HEADER_EFFECTIVE_SIZE;
        rfm69_read(
            rfm,
            RFM69_REG_FIFO,
            &packet[PAYLOAD_BEGIN],
            message_size
        );
        // If this packet would overflow our buffer
        // we choose to just not, you know?

        if (*address != packet[HEADER_TX_ADDRESS]) continue;
        packet_num = packet[HEADER_SEQ_NUMBER];
        if (packet_num < seq_num || packet_num > seq_num_max) continue;

        printf("Received packet: %u\n", packet_num);

        // Account for packet only if it is a new packet
        if (packets_received[packet_num - seq_num]) continue;
        packets_received[packet_num - seq_num] = true;
        num_packets_received++;
        payload_bytes_received += message_size;

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

    printf("Success:\n-RX BYTES:\t%u\n-NUM PACKETS:\t%u\n-PAYLOAD:", payload_bytes_received, num_packets_received);
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
