#include "pico.h"

#include "pico/stdlib.h"

#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/xosc.h"
#include "hardware/rosc.h"
#include "hardware/ticks.h"

#include "hibernate.h"

bool usb_clock_stopped = false;

#if PICO_RP2040
// The RTC clock frequency is 48MHz divided by power of 2 (to ensure an integer
// division ratio will be used in the clocks block).  A divisor of 1024 generates
// an RTC clock tick of 46875Hz.  This frequency is relatively close to the
// customary 32 or 32.768kHz 'slow clock' crystals and provides good timing resolution.
#define RTC_CLOCK_FREQ_HZ       (USB_CLK_HZ / 1024)
#endif

static void start_all_ticks(void) {
    uint32_t cycles = clock_get_hz(clk_ref) / MHZ;
    // Note RP2040 has a single tick generator in the watchdog which serves
    // watchdog, system timer and M0+ SysTick; The tick generator is clocked from clk_ref
    // but is now adapted by the hardware_ticks library for compatibility with RP2350
    // npte: hardware_ticks library now provides an adapter for RP2040

    for (int i = 0; i < (int)TICK_COUNT; ++i) {
        tick_start((tick_gen_num_t)i, cycles);
    }
}

void hibernate_init_clocks(void) {
    // Note: These need setting *before* the ticks are started
    if (running_on_fpga()) {
        for (uint i = 0; i < CLK_COUNT; i++) {
            clock_set_reported_hz(i, 48 * MHZ);
        }
        // clk_ref is 12MHz in both RP2040 and RP2350 FPGA
        clock_set_reported_hz(clk_ref, 12 * MHZ);
        // RP2040 has an extra clock, the rtc
#if HAS_RP2040_RTC
        clock_set_reported_hz(clk_rtc, RTC_CLOCK_FREQ_HZ);
#endif
    } else {
        // Disable resus that may be enabled from previous software
        clocks_hw->resus.ctrl = 0;

        // Enable the xosc
        xosc_init();

        // Before we touch PLLs, switch sys and ref cleanly away from their aux sources.
        hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
        while (clocks_hw->clk[clk_sys].selected != 0x1)
            tight_loop_contents();
        hw_clear_bits(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_BITS);
        while (clocks_hw->clk[clk_ref].selected != 0x1)
            tight_loop_contents();

        /// \tag::pll_init[]
        pll_init(pll_sys, PLL_SYS_REFDIV, PLL_SYS_VCO_FREQ_HZ, PLL_SYS_POSTDIV1, PLL_SYS_POSTDIV2);

// Only touch usb clocks if we are NOT using usb stdio
#if !LIB_PICO_STDIO_USB
        pll_init(pll_usb, PLL_USB_REFDIV, PLL_USB_VCO_FREQ_HZ, PLL_USB_POSTDIV1, PLL_USB_POSTDIV2);
#endif

        /// \end::pll_init[]

        // Configure clocks

        // RP2040 CLK_REF = XOSC (usually) 12MHz / 1 = 12MHz
        // RP2350 CLK_REF = XOSC (XOSC_MHZ) / N (1,2,4) = 12MHz

        // clk_ref aux select is 0 because:
        //
        // - RP2040: no aux mux on clk_ref, so this field is don't-care.
        //
        // - RP2350: there is an aux mux, but we are selecting one of the
        //   non-aux inputs to the glitchless mux, so the aux select doesn't
        //   matter. The value of 0 here happens to be the sys PLL.

        clock_configure_undivided(clk_ref,
                        CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                        0,
                        XOSC_HZ);

        /// \tag::configure_clk_sys[]
        // CLK SYS = PLL SYS (usually) 125MHz / 1 = 125MHz
        clock_configure_undivided(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        SYS_CLK_HZ);
        /// \end::configure_clk_sys[]

#if !LIB_PICO_STDIO_USB
        // CLK USB = PLL USB 48MHz / 1 = 48MHz
        clock_configure_undivided(clk_usb,
                        0, // No GLMUX
                        CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        USB_CLK_HZ);
#endif

        // CLK ADC = PLL USB 48MHZ / 1 = 48MHz
        clock_configure_undivided(clk_adc,
                        0, // No GLMUX
                        CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        USB_CLK_HZ);

#if HAS_RP2040_RTC
        // CLK RTC = PLL USB 48MHz / 1024 = 46875Hz
#if (USB_CLK_HZ % RTC_CLOCK_FREQ_HZ == 0)
        // this doesn't pull in 64 bit arithmetic
        clock_configure_int_divider(clk_rtc,
                        0, // No GLMUX
                        CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        USB_CLK_HZ,
                        USB_CLK_HZ / RTC_CLOCK_FREQ_HZ);

#else
        clock_configure(clk_rtc,
                        0, // No GLMUX
                        CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        USB_CLK_HZ,
                        RTC_CLOCK_FREQ_HZ);

#endif
#endif

        // CLK PERI = clk_sys. Used as reference clock for UART and SPI serial.
        clock_configure_undivided(clk_peri,
                        0,
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                        SYS_CLK_HZ);

#if HAS_HSTX
        // CLK_HSTX = clk_sys. Transmit bit clock for the HSTX peripheral.
        clock_configure_undivided(clk_hstx,
                        0,
                        CLOCKS_CLK_HSTX_CTRL_AUXSRC_VALUE_CLK_SYS,
                        SYS_CLK_HZ);
#endif
    }

    // Finally, all clocks are configured so start the ticks
    // The ticks use clk_ref so now that is configured we can start them
    start_all_ticks();
}

static dormant_source_t _dormant_source;

bool dormant_source_valid(dormant_source_t dormant_source) {
    return (dormant_source == DORMANT_SOURCE_XOSC) || (dormant_source == DORMANT_SOURCE_ROSC);
}

// In order to go into dormant mode we need to be running from a stoppable clock source:
// either the xosc or rosc with no PLLs running. This means we disable the USB and ADC clocks
// and all PLLs
void hibernate_run_from_dormant_source(dormant_source_t dormant_source) {
    assert(dormant_source_valid(dormant_source));
    _dormant_source = dormant_source;

    // FIXME: Just defining average rosc freq here.
    uint src_hz = (dormant_source == DORMANT_SOURCE_XOSC) ? XOSC_HZ : 6.5 * MHZ;
    uint clk_ref_src = (dormant_source == DORMANT_SOURCE_XOSC) ?
                       CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC :
                       CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH;

    // CLK_REF = XOSC or ROSC
    clock_configure(clk_ref,
                    clk_ref_src,
                    0, // No aux mux
                    src_hz,
                    src_hz);

    // CLK SYS = CLK_REF
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                    0, // Using glitchless mux
                    src_hz,
                    src_hz);

#if !LIB_PICO_STDIO_USB
    // CLK USB = 0MHz
    clock_stop(clk_usb);
#endif

    // CLK ADC = 0MHz
    clock_stop(clk_adc);

    // CLK RTC = ideally XOSC (12MHz) / 256 = 46875Hz but could be rosc
    uint clk_rtc_src = (dormant_source == DORMANT_SOURCE_XOSC) ? 
                       CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_XOSC_CLKSRC : 
                       CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH;

    clock_configure(clk_rtc,
                    0, // No GLMUX
                    clk_rtc_src,
                    src_hz,
                    46875);

    // CLK PERI = clk_sys. Used as reference clock for Peripherals. No dividers so just select and enable
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    src_hz,
                    src_hz);

    pll_deinit(pll_sys);
#if !LIB_PICO_STDIO_USB
    pll_deinit(pll_usb);
#endif

    // Assuming both xosc and rosc are running at the moment
    if (dormant_source == DORMANT_SOURCE_XOSC) {
        // Can disable rosc
        rosc_disable();
    } else {
        // Can disable xosc
        xosc_disable();
    }

    // Reconfigure uart with new clocks
    //setup_default_uart();
}

static void _go_dormant(void) {
    assert(dormant_source_valid(_dormant_source));

    if (_dormant_source == DORMANT_SOURCE_XOSC) {
        xosc_dormant();
    } else {
        rosc_set_dormant();
    }
}

void hibernate_goto_dormant_until_pin(uint gpio_pin, bool edge, bool high) {
    bool low = !high;
    bool level = !edge;

    // Configure the appropriate IRQ at IO bank 0
    assert(gpio_pin < NUM_BANK0_GPIOS);

    uint32_t event = 0;

    if (level && low) event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_LOW_BITS;
    if (level && high) event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_LEVEL_HIGH_BITS;
    if (edge && high) event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS;
    if (edge && low) event = IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS;

    gpio_set_dormant_irq_enabled(gpio_pin, event, true);

    _go_dormant();
    // Execution stops here until woken up

    // Clear the irq so we can go back to dormant mode again if we want
    gpio_acknowledge_irq(gpio_pin, event);
}

void hibernate_recover_clocks(uint clock0_orig, uint clock1_orig) {

	// Re-enable ring oscillator 
	rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

	// Reset clocks
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //restart clocks
    hibernate_init_clocks();
}
