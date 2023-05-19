#include "rfm69.h"
#include "stdlib.h"
#include "pico/malloc.h"


struct Rfm69 {
    spi_inst_t* spi; // Initialized SPI instance
    uint pin_cs;
    uint pin_rst;
    RFM69_OP_MODE op_mode;
    int8_t pa_level;
    RFM69_PA_MODE pa_mode;
    uint8_t ocp_trim;
};

RFM69_RETURN rfm69_init(
        Rfm69 **rfm,
        spi_inst_t *spi,
        uint pin_miso,
        uint pin_mosi,
        uint pin_cs,
        uint pin_sck,
        uint pin_rst,
        uint pin_irq_0,
        uint pin_irq_1) 
{
    *rfm = malloc(sizeof(Rfm69));    
    if (rfm == NULL) return RFM69_INIT_MALLOC;

    rfm69_reset(*rfm);

    (*rfm)->spi = spi;
    (*rfm)->pin_cs = pin_cs;
    (*rfm)->pin_rst = pin_rst;
    (*rfm)->op_mode = RFM69_OP_MODE_STDBY;
    (*rfm)->pa_level = RFM69_PA_LEVEL_DEFAULT - 18;
    (*rfm)->pa_mode = RFM69_PA_MODE_PA0;
    (*rfm)->ocp_trim = RFM69_OCP_TRIM_DEFAULT;

    // SPI initialisation. This example will use SPI at 1MHz.
    gpio_set_function(pin_miso, GPIO_FUNC_SPI);
    gpio_set_function(pin_sck,  GPIO_FUNC_SPI);
    gpio_set_function(pin_mosi, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(pin_cs);
    gpio_set_dir(pin_cs, GPIO_OUT);
    gpio_put(pin_cs, 1);

    // Per documentation we leave RST pin floating for at least
    // 10 ms on startup. No harm in waiting 10ms here to
    // guarantee.
    sleep_ms(10); 
    gpio_init(pin_rst);
    gpio_set_dir(pin_rst, GPIO_OUT);
    gpio_put(pin_rst, 0);

    // Try to read version register
    uint8_t buf;
    RFM69_RETURN rval = rfm69_read(*rfm, RFM69_REG_VERSION, &buf, 1);
    if (rval == RFM69_OK) {
        if (buf == 0x00 || buf == 0xFF) { rval = RFM69_INIT_TEST; }
    }

    return rval;
}

void rfm69_reset(Rfm69 *rfm) {
    gpio_put(rfm->pin_rst, 1);
    sleep_us(100);
    gpio_put(rfm->pin_rst, 0);
    sleep_ms(5);
}

// I actually have no idea why this is necessary, but every
// single SPI example in the documentation spends three cycles
// doing nothing before and after the cs pin is set/cleared.
// Likely to allow time for pins to settle?
static inline void cs_select(uint pin_cs) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin_cs, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint pin_cs) {
    asm volatile("nop \n nop \n nop");
    gpio_put(pin_cs, 1);
    asm volatile("nop \n nop \n nop");
}


RFM69_RETURN rfm69_write(
        Rfm69 *rfm, 
        uint8_t address, 
        const uint8_t *src,
        size_t len)
{
    address |= 0x80; // Clear rw bit
    cs_select(rfm->pin_cs);

    int rval = spi_write_blocking(rfm->spi, &address, 1);
    rval += spi_write_blocking(rfm->spi, src, len);

    cs_deselect(rfm->pin_cs);

    if (rval != len + 1)
        return RFM69_SPI_UNEXPECTED_RETURN;

    return RFM69_OK;
}

RFM69_RETURN rfm69_write_masked(
        Rfm69 *rfm, 
        uint8_t address, 
        const uint8_t src,
        const uint8_t mask)
{
    uint8_t reg;
    int rval = rfm69_read(rfm, address, &reg, 1); 
    if (rval == RFM69_SPI_UNEXPECTED_RETURN) return rval;

    reg &= ~mask;
    reg |= src & mask;

    return rfm69_write(rfm, address, &reg, 1);
}

RFM69_RETURN rfm69_read(
        Rfm69 *rfm, 
        uint8_t address, 
        uint8_t *dst,
        size_t len)
{
    address &= 0x7F; // Clear rw bit

    cs_select(rfm->pin_cs);

    RFM69_RETURN rval = spi_write_blocking(rfm->spi, &address, 1);
    rval += spi_read_blocking(rfm->spi, 0, dst, len);

    cs_deselect(rfm->pin_cs);

    if (rval != len + 1)
        return RFM69_SPI_UNEXPECTED_RETURN;

    return RFM69_OK;
}

RFM69_RETURN rfm69_read_masked(
        Rfm69 *rfm,
        uint8_t address,
        uint8_t *dst,
        const uint8_t mask)
{
    RFM69_RETURN rval = rfm69_read(rfm, address, dst, 1);
    *dst &= mask;

    return rval;
}


RFM69_RETURN rfm69_irq1_flag_state(Rfm69 *rfm, RFM69_IRQ1_FLAG flag, bool *state) {
    uint8_t reg;
    RFM69_RETURN rval = rfm69_read_masked(
        rfm,
        RFM69_REG_IRQ_FLAGS_1,
        &reg,
        flag
    );

    if (reg) *state = true;
    else *state = false;

    return rval;
}

RFM69_RETURN rfm69_irq2_flag_state(Rfm69 *rfm, RFM69_IRQ2_FLAG flag, bool *state) {
    uint8_t reg;
    RFM69_RETURN rval = rfm69_read_masked(
        rfm,
        RFM69_REG_IRQ_FLAGS_2,
        &reg,
        flag
    );

    if (reg) *state = true;
    else *state = false;

    return rval;
}

RFM69_RETURN rfm69_frequency_set(Rfm69 *rfm,
                        uint frequency)
{
    // Frf = Fstep * Frf(23,0)
    frequency *= 1000000; // MHz to Hz
    frequency =(frequency / RFM69_FSTEP) + 0.5; // Gives needed register value
    // Split into three bytes.
    uint8_t buf[3] = {
        (frequency >> 16) & 0xFF,
        (frequency >> 8) & 0xFF,
        frequency & 0xFF
    };
    return rfm69_write(rfm, RFM69_REG_FRF_MSB, buf, 3);
}

RFM69_RETURN rfm69_frequency_get(Rfm69 *rfm, uint32_t *frequency) {
        uint8_t buf[3] = {0};
        RFM69_RETURN rval = rfm69_read(rfm, RFM69_REG_FRF_MSB, buf, 3);

        *frequency = (uint32_t) buf[0] << 16;
        *frequency |= (uint32_t) buf[1] << 8;
        *frequency |= (uint32_t) buf[2];
        *frequency *= RFM69_FSTEP;

        return rval;
}

RFM69_RETURN rfm69_bitrate_set(Rfm69 *rfm,
                      uint16_t bit_rate)
{
    uint8_t bytes[2] = {
        (bit_rate & 0xFF00) >> 8,
        bit_rate & 0xFF
    }; 
    return rfm69_write(rfm, RFM69_REG_BITRATE_MSB, bytes, 2);
}

RFM69_RETURN rfm69_bitrate_get(Rfm69 *rfm, uint16_t *bit_rate) {
    uint8_t buf[2] = {0}; 
    RFM69_RETURN rval = rfm69_read(rfm, RFM69_REG_BITRATE_MSB, buf, 2);

    *bit_rate = (uint16_t) buf[0] << 8;
    *bit_rate |= (uint16_t) buf[1];

    return rval;
}

RFM69_RETURN rfm69_mode_set(Rfm69 *rfm, RFM69_OP_MODE mode) {
    RFM69_RETURN rval;

    if (rfm->op_mode != mode) {
        rval = rfm69_write_masked(
            rfm, 
            RFM69_REG_OP_MODE,
            mode,
            RFM69_OP_MODE_MASK
        );
        if (rval == RFM69_OK) {
            rval = rfm69_mode_wait_until_ready(rfm);
            rfm->op_mode = mode;
        }
    } 
    else {
        rval = RFM69_REG_ALREADY_SET; // Already in requested mode
    }

    return rval;
}

void rfm69_mode_get(Rfm69 *rfm, uint8_t *mode) { *mode = rfm->op_mode; }

RFM69_RETURN rfm69_mode_ready(Rfm69 *rfm, bool *ready) {
    return rfm69_irq1_flag_state(rfm, RFM69_IRQ1_FLAG_MODE_READY, ready);
}

RFM69_RETURN rfm69_mode_wait_until_ready(Rfm69 *rfm) {
    RFM69_RETURN rval;

    bool ready = false;
    while (!ready) {
        // Return immediately if there is an spi error
        if ((rval = rfm69_mode_ready(rfm, &ready)) != RFM69_OK)
            break;
    }

    return rval;
}

RFM69_RETURN rfm69_data_mode_set(Rfm69 *rfm, RFM69_DATA_MODE mode) {
    return rfm69_write_masked(
        rfm, 
        RFM69_REG_DATA_MODUL,
        mode,
        RFM69_DATA_MODE_MASK
    );
}

RFM69_RETURN rfm69_data_mode_get(Rfm69 *rfm, uint8_t *mode) {
    return rfm69_read_masked(
        rfm,
        RFM69_REG_DATA_MODUL,
        mode,
        RFM69_DATA_MODE_MASK
    );
}

RFM69_RETURN rfm69_modulation_type_set(Rfm69 *rfm, RFM69_MODULATION_TYPE type) {
    return rfm69_write_masked(
        rfm,
        RFM69_REG_DATA_MODUL,
        type,
        RFM69_MODULATION_TYPE_MASK
    );
}

RFM69_RETURN rfm69_modulation_type_get(Rfm69 *rfm, uint8_t *type) {
    return rfm69_read_masked(
        rfm,
        RFM69_REG_DATA_MODUL,
        type,
        RFM69_MODULATION_TYPE_MASK
    );
}

RFM69_RETURN rfm69_modulation_shaping_set(Rfm69 *rfm, RFM69_MODULATION_SHAPING shaping) {
    return rfm69_write_masked(
        rfm,
        RFM69_REG_DATA_MODUL,
        shaping,
        RFM69_MODULATION_SHAPING_MASK
    );
}

RFM69_RETURN rfm69_modulation_shaping_get(Rfm69 *rfm, uint8_t *shaping) {
    return rfm69_read_masked(
        rfm,
        RFM69_REG_DATA_MODUL,
        shaping,
        RFM69_MODULATION_SHAPING_MASK
    );
}

//reads rssi - see p.68 of rfm69 datasheet
RFM69_RETURN rfm69_rssi_measurment_get(Rfm69 *rfm, int8_t *rssi) {
	uint8_t reg;

	RFM69_RETURN rval = rfm69_read(rfm, RFM69_REG_RSSI_CONFIG, &reg, 1);
    if (rval != RFM69_OK) return rval;

	if(reg != RFM69_RSSI_MEASURMENT_DONE) return RFM69_RSSI_BUSY; //checks RssiDone flag - all other bits should be 0

	rval = rfm69_read(rfm, RFM69_REG_RSSI_VALUE, &reg, 1);

	*rssi = 0 - (reg / 2);

	return rval;
}

//triggers the rfm69 to check rssi
//probably best to run this function before calling rfm69_rssi_get
RFM69_RETURN rfm69_rssi_measurment_start(Rfm69 *rfm) {
	uint8_t reg;

	RFM69_RETURN rval = rfm69_read(rfm, RFM69_REG_RSSI_CONFIG, &reg, 1);
    if (rval != RFM69_OK) return rval;

	reg |= RFM69_RSSI_MEASURMENT_START;

	return rfm69_write(rfm, RFM69_REG_RSSI_CONFIG, &reg, 1);
}

RFM69_RETURN rfm69_power_level_set(Rfm69 *rfm, int8_t pa_level) {
    if (rfm->pa_level == pa_level)
        return RFM69_REG_ALREADY_SET;

    RFM69_RETURN rval;
    uint8_t buf;
    RFM69_PA_MODE pa_mode;
    int8_t pout;

#ifdef RFM69_HIGH_POWER
    bool high_power = true;
#else
    bool high_power = false;
#endif

    // High power modules have to follow slightly different bounds
    // regarding the PA_LEVEL. -2 -> 20 Dbm. 
    // TODO: Make the levels constants
    if (high_power) {
        // Pull pa_level within acceptible bounds 
        if (pa_level < -2)
            pa_level = -2;
        else if (pa_level > 20)
            pa_level = 20;

        // PA1 on only
        if (pa_level < 2) {
            pa_mode = RFM69_PA_MODE_PA1;
            pout = pa_level + 18; 
        }
        // PA1 + PA2
        else if (pa_level < 17) {
            pa_mode = RFM69_PA_MODE_PA1_PA2;
            pout = pa_level + 14; 
        }
        // PA1 + PA2 + High Power Enabled
        else {
            pa_mode = RFM69_PA_MODE_HIGH;
            pout = pa_level + 11; 
        }

    }
    else {
        // Low power modules only use PA0
        pa_mode = RFM69_PA_MODE_PA0;
        // Pull pa_level within acceptible bounds 
        if (pa_level < -18)
            pa_level = -18;
        else if (pa_level > 13)
            pa_level = 13;

        pout = pa_level + 18;
    }

    rval = rfm69_power_mode_set(rfm, pa_mode);
    // If power mode was successfully set (or already set)
    if (rval == RFM69_OK || rval == RFM69_REG_ALREADY_SET) {
        rfm->pa_mode = pa_mode;
        rval = rfm69_write_masked(
            rfm,
            RFM69_REG_PA_LEVEL,
            pout,
            RFM69_PA_OUTPUT_MASK
        );
        if (rval == RFM69_OK)
            rfm->pa_level = pa_level; 
    }

    return rval;
}

RFM69_RETURN rfm69_power_mode_set(Rfm69 *rfm, RFM69_PA_MODE mode) {
    RFM69_RETURN rval;
    uint8_t buf[2];

    // Default settings unless we are in high power mode
    RFM69_HP_CONFIG pa1 = RFM69_HP_PA1_LOW; 
    RFM69_HP_CONFIG pa2 = RFM69_HP_PA2_LOW; 
    RFM69_OCP ocp = RFM69_OCP_ENABLED;
    // Use cached trim
    RFM69_OCP ocp_trim = rfm->ocp_trim;
    
    // Skip if we are already in this mode
    if (rfm->pa_mode == mode)
        rval = RFM69_REG_ALREADY_SET;
    else {
        switch (mode) {
            case RFM69_PA_MODE_PA0:
                buf[0] |= RFM69_PA0_ON;
                break;
            case RFM69_PA_MODE_PA1:
                buf[0] |= RFM69_PA1_ON;
                break;
            case RFM69_PA_MODE_PA1_PA2:
                buf[0] |= RFM69_PA1_ON | RFM69_PA2_ON;
                break;
            case RFM69_PA_MODE_HIGH:
                buf[0] |= RFM69_PA1_ON | RFM69_PA2_ON;
                pa1 = RFM69_HP_PA1_HIGH;
                pa2 = RFM69_HP_PA2_HIGH;
                break;
        } 

        // Set PA pins
        rval = rfm69_write_masked(
            rfm,
            RFM69_REG_PA_LEVEL,
            buf[0],
            RFM69_PA_PINS_MASK
        );
        if (rval == RFM69_OK) {
            // Set into high or low power mode
            buf[0] = pa1;
            buf[1] = pa2;
            rval = rfm69_write(
                rfm,
                RFM69_REG_TEST_PA1,
                buf,
                2
            );
            if (rval == RFM69_OK) {
                rval = rfm69_ocp_set(rfm, ocp);
                if (rval == RFM69_OK) {
                    // If we are entering high power mode, also
                    // set OCP trim to high setting
                    if (pa1 == RFM69_HP_PA1_HIGH)
                        ocp_trim = RFM69_OCP_TRIM_HIGH;
                        
                    rval = rfm69_write_masked(
                        rfm,
                        RFM69_REG_OCP,
                        ocp_trim,
                        RFM69_OCP_TRIM_MASK
                    );
                }
            }
        }
    }

    return rval;
}

RFM69_RETURN rfm69_ocp_set(Rfm69 *rfm, RFM69_OCP state) {
    return rfm69_write_masked(
            rfm,
            RFM69_REG_OCP,
            state,
            RFM69_OCP_ENABLED
    );
}
