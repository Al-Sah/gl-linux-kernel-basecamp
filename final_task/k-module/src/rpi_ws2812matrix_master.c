#include "../rpi_ws2812matrix_master.h"
#include <linux/of.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Final task");
MODULE_VERSION("0.1");

EXPORT_SYMBOL(send_data_to_matrix);
EXPORT_SYMBOL(write_symbols);
EXPORT_SYMBOL(write_pixels);

static int  create_buffer(void **buffer, size_t size);
static void clean_buffer(void **buffer);

static int  init_frame_buffer(void);
static void update_frame_buffer(void* src, size_t pixels);

static void update_brightness_table(void);
static void convert_byte(const uint8_t *in, uint24_t *out);

static int ws2812matrix_probe(struct spi_device *spi); // TODO Move to header
static int ws2812matrix_remove(struct spi_device *spi);

static void on_exit(void);


static struct spi_device *lcd_spi_device = NULL;

static size_t buffer_length = LED_BYTES_COUNT;
static uint8_t *symbols_buffer = NULL;

static struct frame_buffer_t *frame;
// pixels_buffer is a pointer to frame->pixels
static struct pixel_t* pixels_buffer = NULL;


// brightness properties
static int brightness = 100;
module_param(brightness, int, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
static uint8_t *brightness_correction_table;
CLASS_ATTR_RW(brightness);


static struct proc_dir_entry *proc_dir, *proc_info_file;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_read  = proc_read,
};
#else
static struct file_operations proc_fops = {
        .read  = proc_read,
};
#endif


static struct class *led_matrix_controller;
static int major = 0;
static struct cdev led_cdev;
static const struct file_operations dev_fops = {
        .write = dev_write,
};

static const struct spi_device_id ws2812matrix_id_table [] = {
        { "ws2812matrix64", 0 }, // FIXME
        { "rpi_ws2812matrix_master", 0 },
        { }
};
MODULE_DEVICE_TABLE(spi, ws2812matrix_id_table);

static struct spi_driver ws2812matrix_spi_driver = {
        .driver = {
                .name   = "rpi_ws2812matrix_master",
        },
        .probe  = ws2812matrix_probe,
        .remove = ws2812matrix_remove,
        .id_table = ws2812matrix_id_table,
};

static int ws2812matrix_probe(struct spi_device *spi){

    int reset_us = 0, symbol_ns = 0;
    struct device_node *node = spi->dev.of_node;
    lcd_spi_device = spi;
    lcd_spi_device->chip_select = 0;
    lcd_spi_device->max_speed_hz = DEVICE_FREQUENCY;
    lcd_spi_device->mode = SPI_MODE_0;

    of_property_read_u32(node, "reset_us", &reset_us);
    of_property_read_u32(node, "symbol_ns", &symbol_ns);

    pr_info(MODULE_TAG "probe: reset_us : %d; symbol_ns : %d\n", reset_us, symbol_ns);
    pr_info(MODULE_TAG "spi driver probed\n");
    return 0;
}

static int ws2812matrix_remove(struct spi_device *spi){
    // TODO implement clear function
    pr_info(MODULE_TAG "spi driver removed\n");
    return 0;
}


static int __init led_matrix_controller_init(void){

    do{
        ws2812matrix_spi_driver.driver.owner = THIS_MODULE;
        if (spi_register_driver(&ws2812matrix_spi_driver)) {
            printk(KERN_ERR "ws2812matrix_master: failed to register spi driver\n");
            break;
        }
        if(lcd_spi_device == NULL){
            printk(KERN_ERR "ws2812matrix: spi device is not registered\n");
            break;
        }
        if(create_proc_entries() != 0){
            break;
        }
        if(init_sys_interface() != 0){
            break;
        }
        if(init_character_device() != 0){
            break;
        }
        if (create_buffer((void *)&symbols_buffer, LED_BYTES_COUNT)) {
            pr_err(MODULE_TAG "failed to create symbols buffer\n");
        }
        if(init_frame_buffer() != 0){
            break;
        }
        pr_notice(MODULE_TAG " * module loaded *\n");
        return 0;
    } while (42); // to avoid goto

    on_exit();
    pr_err(MODULE_TAG "failed to load\n");
    return -EPERM;
}

static void led_matrix_controller_exit(void){
    on_exit();
    pr_notice(MODULE_TAG " * module uploaded * \n");
}

module_init(led_matrix_controller_init)
module_exit(led_matrix_controller_exit)

static void on_exit(void){
    delete_proc_entries();

    spi_unregister_driver(&ws2812matrix_spi_driver);
    unregister_chrdev_region(MKDEV(major, 0 ), 1 );
    device_destroy(led_matrix_controller, MKDEV(major, 0));
    class_remove_file( led_matrix_controller, &class_attr_brightness);
    class_destroy( led_matrix_controller );
    cdev_del( &led_cdev );

    clean_buffer((void *)&symbols_buffer);
    clean_buffer((void *)&pixels_buffer);
    clean_buffer((void *)&brightness_correction_table);
    if(frame){
        kfree(frame);
    }
    frame = NULL;
}


int init_character_device(void){
    dev_t dev;
    pr_notice(MODULE_TAG "start cdev initialisation\n");

    if((alloc_chrdev_region(&dev, 0, 1, MODULE_NAME)) < 0){
        pr_err("Cannot allocate major number\n");
        return -1;
    } else{
        pr_notice(MODULE_TAG " cdev: major = %d; minor = %d \n",MAJOR(dev), MINOR(dev));
        major = MAJOR(dev);
    }
    cdev_init(&led_cdev, &dev_fops );
    led_cdev.owner = THIS_MODULE;

    if((cdev_add(&led_cdev, dev, 1)) < 0){
        pr_err(MODULE_TAG "failed to add the device to the system\n");
        return -1;
    }
    if((device_create(led_matrix_controller,NULL,dev,NULL,"matrix_pixels")) == NULL){
        pr_err(MODULE_TAG "failed to create device 1\n");
        return -1;
    }
    pr_notice(MODULE_TAG "cdev initialisation: done\n");
    return 0;
}

int create_proc_entries(void){
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }
    proc_info_file = proc_create("info", 0, proc_dir, &proc_fops);
    if (proc_info_file == NULL ) {
        return -EFAULT;
    }
    return 0;
}

void delete_proc_entries(void){
    if (proc_info_file){
        remove_proc_entry("info", proc_dir);
        proc_info_file = NULL;
    }
    if (proc_dir){
        remove_proc_entry(PROC_DIRECTORY, NULL);
        proc_dir = NULL;
    }
}

int init_sys_interface(void){
    if((led_matrix_controller = class_create(THIS_MODULE,MODULE_NAME)) == NULL){
        pr_err(MODULE_TAG "failed to create sys class\n");
        return -1;
    }
    if (class_create_file(led_matrix_controller, &class_attr_brightness) != 0) {
        pr_err(MODULE_TAG "failed to create sys_fs 'brightness' file");
        return -1;
    }
    if(create_buffer((void *)&brightness_correction_table, 256) != 0){
        pr_err(MODULE_TAG "brightness_table: failed to allocate memory\n");
        return -1;
    }
    update_brightness_table();
    return 0;
}

ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset){

    char proc_buffer[256];
    if( *offset != 0 ) {
        return 0;
    }
    snprintf(proc_buffer, 256,
             "\npixels     : %d\n"
             "colours    : %d\n"
             "sequence   : %s\n\n"
             "minimal reset code duration    : %d\n"
             "one symbol transfer time       : %s\n"
             "bits per symbol                : %d\n"
             "device frequency               : %d\n\n",
             PIXELS, 3, "GRB", LED_RESET_US, "1.25", BITS_PER_SYMBOL, DEVICE_FREQUENCY);

    length = strlen(proc_buffer);

    if( copy_to_user( buffer, proc_buffer, length )) {
        pr_warn(MODULE_TAG "failed to copy some chars");
        return -EINVAL;
    }
    *offset = (loff_t)length;
    return (ssize_t)length;
}

ssize_t brightness_show(__attribute__((unused)) struct class *class,
                        __attribute__((unused)) struct class_attribute *attr,
                        char *buf ){

    snprintf(buf, 10, "%d", brightness);
    return (ssize_t)strlen( buf );
}

ssize_t brightness_store(__attribute__((unused)) struct class *class,
                         __attribute__((unused)) struct class_attribute *attr,
                         const char *buf,
                         size_t count ){

    long res;
    if((kstrtol(buf, 10, &res) == 0) && (res >= 0 && res <= 100)){
        brightness = (int)res;
    }else{
        pr_warn(MODULE_TAG "failed to update brightness\n");
    }
    return (ssize_t)count;
}

ssize_t dev_write(
        __attribute__((unused)) struct file *file_p,
        const char __user *buffer, size_t length,
        __attribute__((unused)) loff_t *offset) {

    size_t left;
    frame_buffer_t temp;
    if (length != sizeof(frame_buffer_t)) {
        pr_warn(MODULE_TAG "dev_write: incorrect input; in length: %lu\n", (ulong)length);
        return (ssize_t)length;
    }
    left = copy_from_user(&temp, buffer, length);
    if (left){
        pr_warn(MODULE_TAG "dev_write: failed to write %lu from %lu chars\n", (ulong)left, (ulong)length);
        return (ssize_t)length;
    }
    update_frame_buffer(temp.pixels, temp.len);
    return (ssize_t)length;
}


void validate_brightness(void) {
    int i;
    // Create extra buffer ?
    for (i = 0; i < PIXELS; ++i) { // X * 100 / old_brightness
        frame->pixels[i].colours[RED]   = brightness_correction_table[frame->pixels[i].colours[RED]];
        frame->pixels[i].colours[GREEN] = brightness_correction_table[frame->pixels[i].colours[GREEN]];
        frame->pixels[i].colours[BLUE]  = brightness_correction_table[frame->pixels[i].colours[BLUE]];
    }
}

static void update_frame_buffer(void* src, size_t pixels) {
    size_t left;
    if(pixels > PIXELS){
        pixels = PIXELS; // TODO config if pixels < 64 ...
    }
    left = copy_from_user(frame->pixels, src, pixels*sizeof(pixel_t));
    if (left){
        pr_err(MODULE_TAG "dev_update_frame: failed to write %lu from %lu chars\n",
               (ulong)left, (ulong)frame->len*sizeof(pixel_t));
    }
    validate_brightness();
    convert_pixels_to_symbols();
    send_data_to_matrix();
}


void convert_pixels_to_symbols(void){
    int i, j = 0;
    for(i = 0; i < PIXELS; i++){
        convert_byte(&pixels_buffer[i].colours[RED],   (uint24_t*)(&symbols_buffer[j + 3]));
        convert_byte(&pixels_buffer[i].colours[GREEN], (uint24_t*)(&symbols_buffer[j]));
        convert_byte(&pixels_buffer[i].colours[BLUE],  (uint24_t*)(&symbols_buffer[j + 6]));
        j += 9;
    }
    for(i = PIXELS * PIXEL_COLOURS * 3; i < LED_BYTES_COUNT; ++i){
        symbols_buffer[i] = 0;
    }
}

static void convert_byte(const uint8_t *in, uint24_t *out){
    size_t i;
    uint32_t res = 0;
    uint8_t mask = 1u << 7;
    for(i = 0; i < 8; ++i){
        if((*in & mask) == (uint8_t)0){
            res = res | SYMBOL_LOW;
        } else {
            res = res | SYMBOL_HIGH;
        }
        res = res << 3;
        mask = mask >> 1;
    }
    //out->value = res >> 3; //DEPENDS ON BYTES ORDERING
    res = res >> 3;
    out->value = (res & 0xFF00FF00) | ((res >> 16) & 0x0000FF) | ((res << 16) & 0xFF0000);
}

static int init_frame_buffer(void){
    frame = (frame_buffer_t*)kmalloc(sizeof(frame_buffer_t), GFP_KERNEL);
    if(create_buffer((void *)&pixels_buffer, sizeof(pixel_t) * PIXELS) != 0){
        pr_err(MODULE_TAG "pixels_buffer: failed to allocate memory\n");
        return -1;
    }
    frame->pixels = pixels_buffer;
    return 0;
}

static void update_brightness_table(void){
    int i;
    for(i = 0; i < 255; ++i){
        brightness_correction_table[i] = i * brightness / 100;
    }
}

void write_pixels(uint8_t red, uint8_t green, uint8_t blue){
    int i;
    for(i = 0; i < PIXELS; ++i){
        pixels_buffer[i].colours[RED]   = red;
        pixels_buffer[i].colours[GREEN] = green;
        pixels_buffer[i].colours[BLUE]  = blue;
    }
    convert_pixels_to_symbols();
}

void write_symbols(int colour){
    int i;
    for (i = 0; i < PIXELS * PIXEL_COLOURS * 3;) {
        // converted 24 bits to symbols
        symbols_buffer[i++] = (colour == GREEN) ? 0xDA : 0x92;
        symbols_buffer[i++] = (colour == GREEN) ? 0x4D : 0x49;
        symbols_buffer[i++] = 0x24;

        symbols_buffer[i++] = (colour == RED) ? 0xDA : 0x92;
        symbols_buffer[i++] = (colour == RED) ? 0x4D : 0x49;
        symbols_buffer[i++] = 0x24;

        symbols_buffer[i++] = (colour == BLUE) ? 0xDA : 0x92;
        symbols_buffer[i++] = (colour == BLUE) ? 0x4D : 0x49;
        symbols_buffer[i++] = 0x24;
    }
    for(i = PIXELS * PIXEL_COLOURS * 3; i < LED_BYTES_COUNT; ++i){
        symbols_buffer[i] = 0;
    }
}

int send_data_to_matrix(void){
    if(spi_write(lcd_spi_device, symbols_buffer, buffer_length) != 0){
        pr_err(MODULE_TAG "spi_write failed\n");
        return -1;
    }
    return 0;
}


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