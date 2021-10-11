//
// Created by al_sah on 11.10.21.
//

#ifndef K_MODULE_RPI_WS2812MATRIX_MASTER_H
#define K_MODULE_RPI_WS2812MATRIX_MASTER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "matrix_control.h"

#define PROC_DIRECTORY  "led_matrix_controller"
#define MODULE_NAME     "led_matrix_controller"
#define MODULE_TAG      "[led_matrix_controller]: "


// Symbol definitions
#define SYMBOL_HIGH     0x6     // 1 1 0
#define SYMBOL_LOW      0x4     // 1 0 0
#define BITS_PER_SYMBOL 3       // in SYMBOL_HIGH and SYMBOL_LOW

#define PIXEL_COLOURS   3       // R G B
#define LED_RESET_US    55      // reset time in us (have to be >= 50)


#define PIXELS            64
// 1,25us - time to transfer one symbol (which consists of BITS_PER_SYMBOL)
#define SYMBOL_FREQUENCY 800000  // 1,25us in 1 second  (1_000_000_000 / 1250)
#define DEVICE_FREQUENCY (SYMBOL_FREQUENCY * BITS_PER_SYMBOL)

// Number of bits (zeros) to generate reset code
#define LED_DELAY_BITS  (LED_RESET_US * DEVICE_FREQUENCY / 1000000)
#define LED_DELAY_BYTES (LED_DELAY_BITS / 8 + 1) // +1 to cover fractional part after division

// Number of bytes to cover specified set of leds
#define LED_DATA_BYTES  (PIXELS * PIXEL_COLOURS * BITS_PER_SYMBOL) // instead of x/8 remove BITS_PER_PIXEL
// data bits + reset code
#define LED_BYTES_COUNT (LED_DATA_BYTES + LED_DELAY_BYTES)


int  init_character_device(void);
int  init_spi_device(void);
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

#endif //K_MODULE_RPI_WS2812MATRIX_MASTER_H
