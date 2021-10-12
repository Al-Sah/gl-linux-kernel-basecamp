//
// Created by al_sah on 11.10.21.
//

#ifndef K_MODULE_WS2812_CONTROLLER_H
#define K_MODULE_WS2812_CONTROLLER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>

#include "matrix_control.h"

#define PROC_DIRECTORY  "ws2812_controller"
#define MODULE_NAME     "ws2812_controller"
#define MODULE_TAG      "[ws2812_controller]: "

enum ws2812_symbols{
    SYMBOL_HIGH = 0x6, // 1 1 0
    SYMBOL_LOW = 0x4,  // 1 0 0
};

struct ws2812_t{
    char* sequence; // rgb or gbr
    uint8_t bits_per_symbol;
    uint8_t symbol_high;
    uint8_t symbol_low;
    uint32_t pixels;
    uint32_t colours;
    uint32_t reset_us;
    uint32_t symbol_ns;
    uint32_t frequency;
};

uint32_t get_symbols_buffer_size(struct ws2812_t *device);

int init_spi_ws2812_controller(void);
int ws2812_controller_probe(struct spi_device *spi);
int ws2812_controller_remove(struct spi_device *spi);

int  init_character_device(void);
int  init_sys_interface(void);
int  create_proc_entries(void);
void delete_proc_entries(void);


void write_symbols(int colour);
void write_pixels(uint8_t red, uint8_t green, uint8_t blue);


void validate_brightness(void);
void convert_pixels_to_symbols(void);
int  send_data_to_matrix(void);


ssize_t brightness_show(__attribute__((unused)) struct class *class,
       __attribute__((unused)) struct class_attribute *attr,
       char *buf);

ssize_t brightness_store(__attribute__((unused)) struct class *class,
        __attribute__((unused)) struct class_attribute *attr,
        const char *buf,
        size_t count);


ssize_t proc_read(__attribute__((unused)) struct file *file_p,
        char __user *buffer,
        size_t length,
        loff_t *offset);

ssize_t dev_write(__attribute__((unused)) struct file *file_p,
        const char __user *buffer,
        size_t length,
        __attribute__((unused)) loff_t *offset);

#endif //K_MODULE_WS2812_CONTROLLER_H
