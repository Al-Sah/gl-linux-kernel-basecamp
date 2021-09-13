#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Kernel module to flip all words in a str and do it uppercase");
MODULE_VERSION("0.1");


static char *str = "Hello world";
module_param(str, charp, 0000);


#define MODULE_TAG "string_processor"


static int __init string_processor_init(void)
{
    return -1;
}

module_init(string_processor_init)
