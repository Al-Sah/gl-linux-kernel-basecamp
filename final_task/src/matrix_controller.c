#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Final task");
MODULE_VERSION("0.1");

//#define MODULE_ID   "led_matrix_controller"
#define MODULE_TAG  "[led_matrix_controller]: " // For the printk

static struct spi_device *lcd_spi_device = NULL;
static uint8_t *spi_data_buffer = NULL;

// Symbol definitions
#define SYMBOL_HIGH     0x6     // 1 1 0
#define SYMBOL_LOW      0x4     // 1 0 0
#define BITS_PER_SYMBOL 3       // in SYMBOL_HIGH and SYMBOL_LOW

#define PIXEL_COLOURS   3       // R G B
#define BITS_PER_PIXEL  8       // sizeof(uint8_t) in bits - describes one color
#define LED_RESET_US    55      // reset time in us (have to be >= 50)


#define LEDS            64
// 1,25us - time to transfer one symbol (which consists of BITS_PER_SYMBOL)
#define SYMBOL_FREQUENCY 800000  // 1,25us in 1 second  (1_000_000_000 / 1250)
#define DEVICE_FREQUENCY (SYMBOL_FREQUENCY * BITS_PER_SYMBOL)

// Number of bits (zeros) to generate reset code
#define LED_DELAY_BITS  (LED_RESET_US * DEVICE_FREQUENCY / 1000000)
#define LED_DELAY_BYTES (LED_DELAY_BITS / 8 + 1) // +1 to cover fractional part after division

// Number of bits to cover specified set of leds
#define LED_DATA_BITS   (LEDS  * PIXEL_COLOURS * BITS_PER_PIXEL * BITS_PER_SYMBOL)
#define LED_DATA_BYTES  (LEDS * PIXEL_COLOURS * BITS_PER_SYMBOL) // instead of x/8 remove BITS_PER_PIXEL

// data bits + reset code
#define LED_BITS_COUNT  (LED_DATA_BITS + LED_DELAY_BITS)
#define LED_BYTES_COUNT (LED_DATA_BYTES + LED_DELAY_BYTES)


/*struct led_t {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};*/

enum colours{
    GREEN = 1,
    RED = 2,
    BLUE = 3,
    NONE = 4,
};

static int create_buffer(void **buffer, size_t size);
static void clean_buffer(void **buffer);

// is used for test
static void test_fill_spi_buffer(int colour){
    int i;
    for (i = 0; i < LEDS * PIXEL_COLOURS * 3;) {
        // converted 24 bits to symbols
        spi_data_buffer[i++] = (colour == GREEN) ? 0xDA : 0x92;
        spi_data_buffer[i++] = (colour == GREEN) ? 0x4D : 0x49;
        spi_data_buffer[i++] = 0x24;

        spi_data_buffer[i++] = (colour == RED) ? 0xDA : 0x92;
        spi_data_buffer[i++] = (colour == RED) ? 0x4D : 0x49;
        spi_data_buffer[i++] = 0x24;

        spi_data_buffer[i++] = (colour == BLUE) ? 0xDA : 0x92;
        spi_data_buffer[i++] = (colour == BLUE) ? 0x4D : 0x49;
        spi_data_buffer[i++] = 0x24;
    }

    for(i = LEDS * PIXEL_COLOURS * 3; i < LED_BYTES_COUNT; ++i){
        spi_data_buffer[i] = 0;
    }
}

static int test_spi_write(void){
    size_t length = LED_BYTES_COUNT;
    test_fill_spi_buffer(RED);
    if(spi_write(lcd_spi_device, spi_data_buffer, length) != 0){
        pr_err(MODULE_TAG "spi_write failed\n");
        return -1;
    }
    ssleep(1);
    test_fill_spi_buffer(GREEN);
    if(spi_write(lcd_spi_device, spi_data_buffer, length) != 0){
        pr_err(MODULE_TAG "spi_write failed\n");
        return -1;
    }
    ssleep(1);
    test_fill_spi_buffer(BLUE);
    if(spi_write(lcd_spi_device, spi_data_buffer, length) != 0){
        pr_err(MODULE_TAG "spi_write failed\n");
        return -1;
    }
    ssleep(1);
    test_fill_spi_buffer(NONE);
    if(spi_write(lcd_spi_device, spi_data_buffer, length) != 0){
        pr_err(MODULE_TAG "spi_write failed\n");
        return -1;
    }
    return 0;
}


static void inline on_exit(void){
    if (lcd_spi_device) {
        spi_unregister_device(lcd_spi_device);
    }
    clean_buffer((void *)&spi_data_buffer);
}

static int init_spi_device(void){
    int ret;
    struct spi_master *master;
    struct spi_board_info led_matrix_info = {
            .modalias = "LED_MATRIX",
            .max_speed_hz = DEVICE_FREQUENCY, // time to transfer one bit
            .bus_num = 0,
            .chip_select = 0,
            .mode = SPI_MODE_0,
    };

    master = spi_busnum_to_master(led_matrix_info.bus_num);
    if (!master) {
        pr_err(MODULE_TAG "failed to find master; check if SPI enabled\n");
        return -1;
    }
    lcd_spi_device = spi_new_device(master, &led_matrix_info);
    if (!lcd_spi_device) {
        pr_err(MODULE_TAG "failed to create slave\n");
        return -1;
    }
    ret = spi_setup(lcd_spi_device);
    if (ret) {
        pr_err(MODULE_TAG "failed to setup slave\n");
        return ret;
    }
    ret = create_buffer((void *)&spi_data_buffer, LED_BYTES_COUNT);
    if (ret) {
        pr_err(MODULE_TAG "failed to create buffer\n");
    }
    return ret;
}

static int __init led_matrix_controller_init(void){
    int ret;

    do{
        ret = init_spi_device();
        if (ret != 0) {
            break;
        }

        ret = test_spi_write();
        if (ret != 0) {
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



static int create_buffer(void **buffer, size_t size){
    *buffer = kmalloc(size, GFP_KERNEL);
    if (*buffer == NULL){
        pr_err(MODULE_TAG "failed to create buffer");
        return -1;
    }
    return 0;
}

static void clean_buffer(void **buffer){
    if (*buffer) {
        kfree(*buffer);
        *buffer = NULL;
    }
}