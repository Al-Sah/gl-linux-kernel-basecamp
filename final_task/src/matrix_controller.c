#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Final task");
MODULE_VERSION("0.1");

//#define MODULE_ID   "led_matrix_controller"
#define MODULE_TAG  "[led_matrix_controller]: " // For the printk

static struct spi_device *lcd_spi_device = NULL;


// Symbol definitions
#define SYMBOL_HIGH     0x6     // 1 1 0
#define SYMBOL_LOW      0x4     // 1 0 0
#define BITS_PER_SYMBOL 3       // in SYMBOL_HIGH and SYMBOL_LOW

#define LED_COLOURS     3       // R G B
#define BITS_PER_PIXEL  8       // sizeof(uint8_t) - describes one color
#define LED_RESET_US    55      // reset time in us (have to be >= 50)
#define LED_FREQUENCY   800000

// Number of bits (zeros) to generate reset code
#define LED_DELAY_BITS(frequency) ((LED_RESET_US * ((frequency) * 3)) / 1000000)
// Number of bits to cover specified set of leds
#define LED_DATA_BITS(leds) ((leds) * LED_COLOURS * BITS_PER_PIXEL * BITS_PER_SYMBOL)
// data bits + reset code
#define LED_BITS_COUNT(leds, frequency) (LED_DATA_BITS(leds) + LED_DELAY_BITS(frequency))


struct led_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};


static void inline on_exit(void){
    if (lcd_spi_device) {
        spi_unregister_device(lcd_spi_device);
    }
}

static int __init led_matrix_controller_init(void){
    int ret;
    struct spi_master *master;
    struct spi_board_info led_matrix_info = {
            .modalias = "LED_MATRIX",
            .max_speed_hz = LED_FREQUENCY * BITS_PER_SYMBOL, // time to transfer one bit
            .bus_num = 0,
            .chip_select = 0,
            .mode = SPI_MODE_0,
    };

    do{
        master = spi_busnum_to_master(led_matrix_info.bus_num);
        if (!master) {
            pr_err(MODULE_TAG "failed to find master; check if SPI enabled\n");
            break;
        }
        lcd_spi_device = spi_new_device(master, &led_matrix_info);
        if (!lcd_spi_device) {
            pr_err(MODULE_TAG "failed to create slave\n");
            break;
        }
        ret = spi_setup(lcd_spi_device);
        if (ret) {
            pr_err(MODULE_TAG "failed to setup slave\n");
            spi_unregister_device(lcd_spi_device);
            break;
        }

        pr_notice(MODULE_TAG "spi device setup completed\n");
        pr_notice(MODULE_TAG "loaded\n");
        return 0;
    } while (42); // to avoid goto

    on_exit();
    pr_err(MODULE_TAG "failed to load\n");
    return -EPERM;
}

static void led_matrix_controller_exit(void){
    on_exit();
    pr_notice(MODULE_TAG "uploaded\n");
}

module_init(led_matrix_controller_init)
module_exit(led_matrix_controller_exit)