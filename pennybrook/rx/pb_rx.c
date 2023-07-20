#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/timer.h"

#include "ssd1306.h"

#include "rfm69.h" 
#include "rudp.h"
#include "common_config.h"

#include "rtc.h"
#include "f_util.h"
#include "hw_config.h"

#define ever ;; 

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20
#define PIN_IRQ_0  21
#define PIN_IRQ_1  21

void set_bi() {
    bi_decl(bi_program_name("Penny Brook Test Receiver"));
    bi_decl(bi_program_description("WISDOM sensor network pennybrook rx test."))
    bi_decl(bi_1pin_with_name(PIN_MISO, "MISO"));
    bi_decl(bi_1pin_with_name(PIN_CS, "CS"));
    bi_decl(bi_1pin_with_name(PIN_SCK, "SCK"));
    bi_decl(bi_1pin_with_name(PIN_MOSI, "MOSI"));
    bi_decl(bi_1pin_with_name(PIN_RST, "RST"));
    //bi_decl(bi_1pin_with_name(PIN_IRQ_0, "IRQ 0"));
    //bi_decl(bi_1pin_with_name(PIN_IRQ_1, "IRQ 1"));
}

int main() {

    absolute_time_t time_since_boot;
    uint32_t ms_since_start;
    char time_string[16];
    char str[128];

    // Set Picotool binary info
    set_bi();
    stdio_init_all(); // To be able to use printf
    time_init();

    sd_card_t *sd = sd_get_by_num(0);
    FIL fil;
    FRESULT fr;
    UINT bw;
    BYTE work[FF_MAX_SS];
    const char* const filename = "test_log_file.csv";

    spi_init(SPI_PORT, 1000*1000); // Defaults to master mode, which we want

    fr = f_mount(&sd->fatfs, sd->pcName, 1);
    if(fr != FR_OK) {
	    printf("error mounting sd card!\n");
	    printf("%s\n", FRESULT_str(fr));
    }

    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if(fr != FR_OK) {
	    printf("error opening file\n");
	    printf("%s\n", FRESULT_str(fr));
    }

    time_since_boot = get_absolute_time();
    ms_since_start = to_ms_since_boot(time_since_boot);
    f_printf(&fil, "\n#Starting log; uptime:  %dms\n", ms_since_start);

    f_close(&fil);
    
    Rfm69 *rfm = rfm69_create();
    rfm69_rudp_init(
        rfm,
        SPI_PORT,
        PIN_MISO,
        PIN_MOSI,
        PIN_CS,
        PIN_SCK,
        PIN_RST,
        PIN_IRQ_0,
        PIN_IRQ_1
    );

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
	common_radio_config(rfm);

    rfm69_node_address_set(rfm, 0x02); 

    rfm69_power_level_set(rfm, 0);
    TrxReport report;
    bool success;
	float *teros_buf;
    for(ever) { 

        uint8_t address;
        uint size = 100000;
        uint8_t payload[size];

        printf("Receiving...\n");

        success = rfm69_rudp_receive(
                rfm,
                &report,
                &address,
                payload,
                &size,
                12000,
                2000
        );

        printf("Report\n");
        printf("------\n");
        printf("      tx_address: %u\n", report.tx_address);
        printf("      rx_address: %u\n", report.rx_address);
        printf("  bytes_expected: %u\n", report.payload_size);
        printf("  bytes_received: %u\n", report.bytes_received);
        printf("packets_received: %u\n", report.data_packets_received);
        printf("       acks_sent: %u\n", report.acks_sent);
        printf("      racks_sent: %u\n", report.racks_sent);
        printf("   rack_requests: %u\n", report.rack_requests_received);

        switch(report.return_status) {
            case RUDP_OK:
                printf("   return_status: RUDP_OK\n");
                break;
            case RUDP_TIMEOUT:
                printf("   return_status: RUDP_TIMEOUT\n");
                break;
            case RUDP_BUFFER_OVERFLOW:
                printf("   return_status: RUDP_BUFFER_OVERFLOW\n");
                break;
        }


        if (success) {
            uint num_blinks = 3;
            for(; num_blinks; num_blinks--) {
                gpio_put(PICO_DEFAULT_LED_PIN, 1);
                sleep_ms(100);
                gpio_put(PICO_DEFAULT_LED_PIN, 0);
                sleep_ms(50);
            }

			teros_buf = (float *)payload;
			printf("vwc: %.6f\n", teros_buf[0]);
			printf("temp: %.6f\n", teros_buf[1]);
			printf("\n");

			f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
			time_since_boot = get_absolute_time();
			ms_since_start = to_ms_since_boot(time_since_boot);
			//format: TIMESTAMP, VWC, TEMP
			f_printf(&fil, "%d, %.6f, %.6f\n", ms_since_start, teros_buf[0], teros_buf[1]);
			f_close(&fil);

			
			sleep_ms(60000);
        }

    }
    
    return 0;
}
