#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define OLED_WIDTH 128
#define OLED_HEIGHT 32


int main()
{

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(500);

    ssd1306_t oled;
    oled.external_vcc = 0;
    ssd1306_init(&oled, OLED_WIDTH, OLED_HEIGHT, 0x3c, I2C_PORT);
    ssd1306_poweron(&oled);
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 0, 1, "meow");
    ssd1306_show(&oled);

    return 0;
}
