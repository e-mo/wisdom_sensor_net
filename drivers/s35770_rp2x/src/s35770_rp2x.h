#ifndef S35770_RP2X_H
#define S35770_RP2X_H

#define S35770_I2C_ADDRESS (0x32)

typedef unsigned uint;

typedef struct s35770_inst_s {
	uint i2c_inst;
	uint pin_reset;
} s35770_context_t;

void s35770_rp2x_init(s35770_context_t *counter, uint i2c_inst, uint pin_reset);

void s35770_rp2x_reset(s35770_context_t *counter);

bool s35770_rp2x_read(s35770_context_t *counter, uint32_t *count);

#endif // S35770_RP2X_H
