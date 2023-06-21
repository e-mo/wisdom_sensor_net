#include "rudp.h"
#include "string.h"

typedef struct _seq_byte_manifest {
    uint8_t manifest[TX_PACKETS_MAX];

    uint8_t num_expected;
    uint8_t num_received;

    uint8_t seq_begin;
    uint8_t seq_end;
    uint8_t next_expected;

    uint8_t missing_buffer_0[TX_PACKETS_MAX];
    uint8_t num_missing_0;

    uint8_t missing_buffer_1[TX_PACKETS_MAX];
    uint8_t num_missing_1;

    uint8_t active_buffer_num;
    uint8_t *active_buffer;
    uint8_t *active_buffer_count;
    uint8_t *off_buffer;
    uint8_t *off_buffer_count;

    uint8_t next_missing_expected;

    uint8_t manifest_fresh;    // Is this our first time through the sequence?
    uint8_t manifest_complete; // Have we completed the manifest?

} sb_manifest_t;



// Ensures all values are predictably initialized
void packet_manifest_init(packet_manifest_t *sbm, uint8_t num_expected, uint8_t seq_begin) {
    sbm->num_expected = num_expected; 
    sbm->num_received = 0; 

    sbm->seq_begin = seq_begin;
    sbm->seq_end = seq_begin + num_expected - 1;

    sbm->next_expected = seq_begin;

    memset(sbm->manifest, false, TX_PACKETS_MAX);

    sbm->num_missing_0 = 0;
    sbm->num_missing_1 = 0;

    sbm->active_buffer_num - 0;
    sbm->active_buffer = sbm->missing_buffer_0;
    sbm->num_missing = &sbm->num_missing_0;
    sbm->off_buffer = NULL;
    sbm->off_buffer_count = NULL;

    sbm->next_missing_expected = 0;

    sbm->manifest_fresh = true;
    sbm->manifest_complete = false;
}

bool sb_manifest_add(packet_manifest_t *sbm, uint8_t packet_num) {
    if (sbm->manifest[packet_num]) return false;
    if (packet_num > sbm->seq_begin || packet_num < sbm->seq_end) return false;
}
