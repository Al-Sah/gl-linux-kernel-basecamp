#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include "../matrix_control.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Testing module for the matrix_controller");
MODULE_VERSION("0.1");

#define MODULE_TAG  "[matrix_controller_tester]: "


extern int  send_data_to_matrix(void);
extern void write_symbols(int colour);
extern void write_pixels(uint8_t red, uint8_t green, uint8_t blue);

static int test_symbols_layout(void);
static int test_pixels_layout(void);


static int test_symbols_layout(void){
    write_symbols(RED);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);

    write_symbols(GREEN);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);

    write_symbols(BLUE);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);

    write_symbols(NONE);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);
    return 0;
}

static int test_pixels_layout(void){

    write_pixels(72, 209, 204);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);

    write_pixels(52, 176, 30);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);

    write_pixels(20, 40, 200);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);

    write_pixels(0, 0, 0);
    if(send_data_to_matrix() != 0){
        return -1;
    }
    msleep(500);
    return 0;
}

static int __init mod_test_init(void){
    int ret;
    pr_notice(MODULE_TAG "start testing\n");
    ret = test_symbols_layout();
    if (ret != 0) {
        pr_notice(MODULE_TAG "test_symbols_layout: failure\n");
    }
    ret = test_pixels_layout();
    if (ret != 0) {
        pr_notice(MODULE_TAG "test_pixels_layout: failure\n");
    }
    pr_notice(MODULE_TAG "Tests are completed\n");
    return -1;
}
module_init(mod_test_init)