#include "../ws2812_controller.h"

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

static void on_exit(void);

#define SUCCESS_CODE 0
#define FAILURE_CODE (-1)

static struct ws2812_t ws2812_device = {};
static struct spi_device *ws2812_spi_device = NULL;

static size_t symbols_buffer_length = 0;
static uint8_t *symbols_buffer = NULL;

static struct frame_buffer_t *frame = NULL;
// pixels_buffer is a pointer to frame->pixels
static struct pixel_t* pixels_buffer = NULL;

int pixels_as_param = -1;
module_param_named(pixels, pixels_as_param, int, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));

// brightness properties
int brightness = 100;
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


static struct class *ws2812_controller_class;
static int major = 0;
static struct cdev ws2812_controller_cdev;
static const struct file_operations dev_fops = {
        .write = dev_write,
};

static const struct spi_device_id ws2812_id_table [] = {
        { "ws2812_controller", 0 },
        { }
};
MODULE_DEVICE_TABLE(spi, ws2812_id_table);

static struct spi_driver spi_ws2812_controller = {
        .driver = {
                .name   = "ws2812_controller",
        },
        .probe  = ws2812_controller_probe,
        .remove = ws2812_controller_remove,
        .id_table = ws2812_id_table,
};

static int __init ws2812_controller_init(void){

    do{
        if(init_spi_ws2812_controller() != SUCCESS_CODE){
            break;
        }
        if(create_proc_entries() != SUCCESS_CODE){
            break;
        }
        if(init_sys_interface() != SUCCESS_CODE){
            break;
        }
        if(init_character_device() != SUCCESS_CODE){
            break;
        }
        symbols_buffer_length = get_symbols_buffer_size(&ws2812_device);
        if (create_buffer((void *)&symbols_buffer, symbols_buffer_length)) {
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

static void ws2812_controller_exit(void){
    on_exit();
    pr_notice(MODULE_TAG " * module uploaded * \n");
}

module_init(ws2812_controller_init)
module_exit(ws2812_controller_exit)

static void on_exit(void){
    delete_proc_entries();

    spi_unregister_driver(&spi_ws2812_controller);
    unregister_chrdev_region(MKDEV(major, 0 ), 1 );
    device_destroy(ws2812_controller_class, MKDEV(major, 0));
    class_remove_file(ws2812_controller_class, &class_attr_brightness);
    class_destroy(ws2812_controller_class );
    cdev_del( &ws2812_controller_cdev );

    clean_buffer((void *)&symbols_buffer);
    clean_buffer((void *)&pixels_buffer);
    clean_buffer((void *)&brightness_correction_table);
    if(frame){
        kfree(frame);
    }
    frame = NULL;
}

int init_spi_ws2812_controller(void){
    spi_ws2812_controller.driver.owner = THIS_MODULE;
    if (spi_register_driver(&spi_ws2812_controller)) {
        pr_err(MODULE_TAG "failed to register spi driver\n");
        return FAILURE_CODE;
    }
    if(ws2812_spi_device == NULL){
        pr_err(MODULE_TAG "ws2812_device is not registered\n");
        return FAILURE_CODE;
    }
    pr_notice(MODULE_TAG "spi_driver and spi_device are registered\n");
    return SUCCESS_CODE;
}

int ws2812_controller_probe(struct spi_device *spi){

    struct device_node *node = spi->dev.of_node;

    if( strcmp(node->name, "ws2812matrix64") != 0){
        pr_info(MODULE_TAG "ws2812 probe: unsupported device: %s\n", node->name);
        return 0;
    }

    pr_info(MODULE_TAG "ws2812 probe: device: %s\n", node->name);
    of_property_read_u32(node, "reset_us", &ws2812_device.reset_us);
    of_property_read_u32(node, "symbol_ns", &ws2812_device.symbol_ns);
    of_property_read_u32(node, "colours", &ws2812_device.colours);
    of_property_read_u32(node, "pixels", &ws2812_device.pixels);
    of_property_read_string(node, "sequence", (const char **) &ws2812_device.sequence);

    if(pixels_as_param != -1){
        ws2812_device.pixels = pixels_as_param;
    }
    ws2812_device.bits_per_symbol = 3;
    ws2812_device.symbol_high = SYMBOL_HIGH;
    ws2812_device.symbol_low = SYMBOL_LOW;
    ws2812_device.frequency = spi->max_speed_hz;
    ws2812_spi_device = spi;
    ws2812_spi_device->chip_select = 0;
    ws2812_spi_device->mode = SPI_MODE_0;

    pr_info(MODULE_TAG "spi_ws2812_controller: probed\n");
    return 0;
}
uint32_t get_symbols_buffer_size(struct ws2812_t *device){
    return (device->reset_us * device->frequency / 1000000 / 8 + 1) + // DELAY_BYTES
           (device->pixels * device->colours * device->bits_per_symbol);     // SYMBOLS_BYTES
}

int ws2812_controller_remove(struct spi_device *spi){
    write_symbols(NONE);
    if(spi_write(spi, symbols_buffer, symbols_buffer_length) != 0){
        pr_err(MODULE_TAG "spi_write: failed\n");
    }
    pr_info(MODULE_TAG "spi_ws2812_controller removed\n");
    return SUCCESS_CODE;
}

int init_character_device(void){
    dev_t dev;
    pr_notice(MODULE_TAG "cdev initialisation ... \n");

    if((alloc_chrdev_region(&dev, 0, 1, MODULE_NAME)) < 0){
        pr_err(MODULE_TAG "cannot allocate major number\n");
        return -1;
    } else{
        pr_notice(MODULE_TAG "cdev: major = %d; minor = %d \n",MAJOR(dev), MINOR(dev));
        major = MAJOR(dev);
    }
    cdev_init(&ws2812_controller_cdev, &dev_fops );
    ws2812_controller_cdev.owner = THIS_MODULE;

    if((cdev_add(&ws2812_controller_cdev, dev, 1)) < 0){
        pr_err(MODULE_TAG "failed to add the device to the system\n");
        return FAILURE_CODE;
    }
    if(ws2812_controller_class == NULL){
        pr_err(MODULE_TAG "ws2812_controller_class is not initialized\n");
        return FAILURE_CODE;
    }
    if((device_create(ws2812_controller_class, NULL, dev, NULL, "matrix_pixels")) == NULL){
        pr_err(MODULE_TAG "failed to create ws2812_controller_cdev\n");
        return FAILURE_CODE;
    }
    pr_notice(MODULE_TAG "cdev initialisation: done\n");
    return SUCCESS_CODE;
}

int create_proc_entries(void){
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        pr_err(MODULE_TAG "failed to create proc directory\n");
        return -EFAULT;
    }
    proc_info_file = proc_create("info", 0, proc_dir, &proc_fops);
    if (proc_info_file == NULL ) {
        pr_err(MODULE_TAG "failed to create proc information file\n");
        return -EFAULT;
    }
    return SUCCESS_CODE;
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
    if((ws2812_controller_class = class_create(THIS_MODULE, MODULE_NAME)) == NULL){
        pr_err(MODULE_TAG "failed to create sys class\n");
        return FAILURE_CODE;
    }
    if (class_create_file(ws2812_controller_class, &class_attr_brightness) != SUCCESS_CODE) {
        pr_err(MODULE_TAG "failed to create sys_fs 'brightness' file");
        return FAILURE_CODE;
    }
    if(create_buffer((void *)&brightness_correction_table, 256) != SUCCESS_CODE){
        pr_err(MODULE_TAG "brightness_table: failed to allocate memory\n");
        return FAILURE_CODE;
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
             "minimal reset code duration    : %u us\n"
             "one symbol transfer time       : %u ns\n"
             "bits per symbol                : %u\n"
             "device frequency               : %u HZ\n\n",
             ws2812_device.pixels,
             ws2812_device.colours,
             ws2812_device.sequence,
             ws2812_device.reset_us,
             ws2812_device.symbol_ns,
             ws2812_device.bits_per_symbol,
             ws2812_device.frequency);

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
        update_brightness_table();
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
    for (i = 0; i < ws2812_device.pixels; ++i) {
        frame->pixels[i].colours[RED]   = brightness_correction_table[frame->pixels[i].colours[RED]];
        frame->pixels[i].colours[GREEN] = brightness_correction_table[frame->pixels[i].colours[GREEN]];
        frame->pixels[i].colours[BLUE]  = brightness_correction_table[frame->pixels[i].colours[BLUE]];
    }
}

static void update_frame_buffer(void* src, size_t pixels) {
    size_t left;
    if(pixels > ws2812_device.pixels){
        pixels = ws2812_device.pixels; // TODO config if pixels < 64 ...
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
    size_t i, j = 0;
    uint32_t bytes = get_symbols_buffer_size(&ws2812_device);
    for(i = 0; i < ws2812_device.pixels; i++){
        convert_byte(&pixels_buffer[i].colours[RED],   (uint24_t*)(&symbols_buffer[j + 3]));
        convert_byte(&pixels_buffer[i].colours[GREEN], (uint24_t*)(&symbols_buffer[j]));
        convert_byte(&pixels_buffer[i].colours[BLUE],  (uint24_t*)(&symbols_buffer[j + 6]));
        j += 9;
    }
    for(i = ws2812_device.pixels * ws2812_device.colours * 3; i < bytes; ++i){
        symbols_buffer[i] = 0;
    }
}

static void convert_byte(const uint8_t *in, uint24_t *out){
    size_t i;
    uint32_t res = 0;
    uint8_t mask = 1u << 7;
    for(i = 0; i < 8; ++i){
        if((*in & mask) == (uint8_t)0){
            res = res | ws2812_device.symbol_low;
        } else {
            res = res | ws2812_device.symbol_high;
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
    if(create_buffer((void *)&pixels_buffer, sizeof(pixel_t) * ws2812_device.pixels) != 0){
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
    for(i = 0; i < ws2812_device.pixels; ++i){
        pixels_buffer[i].colours[RED]   = red;
        pixels_buffer[i].colours[GREEN] = green;
        pixels_buffer[i].colours[BLUE]  = blue;
    }
    validate_brightness();
    convert_pixels_to_symbols();
}

void write_symbols(int colour){
    size_t i;
    uint32_t bytes = get_symbols_buffer_size(&ws2812_device);
    for (i = 0; i <  ws2812_device.pixels * ws2812_device.colours * 3;) {
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
    for(i = ws2812_device.pixels * ws2812_device.colours * 3; i < bytes; ++i){
        symbols_buffer[i] = 0;
    }
}

int send_data_to_matrix(void){
    if(spi_write(ws2812_spi_device, symbols_buffer, symbols_buffer_length) != 0){
        pr_err(MODULE_TAG "spi_write failed\n");
        return -1;
    }
    return 0;
}


static int create_buffer(void **buffer, size_t size){
    *buffer = kmalloc(size, GFP_KERNEL);
    if (*buffer == NULL){
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

static void clean_buffer(void **buffer){
    if (*buffer) {
        kfree(*buffer);
        *buffer = NULL;
    }
}