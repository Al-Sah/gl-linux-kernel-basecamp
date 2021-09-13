#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Kernel module to flip all words in a str and do it uppercase");
MODULE_VERSION("0.1");


static char str[256] = "Hello world";
module_param_string(str, str, 256, 0);

#define MODULE_TAG "string_processor"

static void to_uppercase(char* src){
    int i;
    for (i = 0; i < strlen(src); ++i) {
        if (src[i] >= 97 && src[i] <= 122) // if the char in is the range a-z
        {
            src[i] -= 32; // convert to uppercase
        }
    }
}

static int __init string_processor_init(void)
{
    char string_to_modify[64];
    strcpy(string_to_modify, str);
    printk(KERN_INFO MODULE_TAG": Input str: %s\n", string_to_modify);
    to_uppercase(string_to_modify);
    printk(KERN_INFO MODULE_TAG": Output str: %s\n", string_to_modify);
    return -1;
}

module_init(string_processor_init)
