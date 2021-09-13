#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Kernel module to flip all words in a str and do it uppercase");
MODULE_VERSION("0.1");


static char str[256] = "Hello world";
module_param_string(str, str, 256, 0);

#define MODULE_TAG "string_processor"

/// @param src have to be a zero-terminated string
static void to_uppercase(char* src){
    int i;
    for (i = 0; i < strlen(src); ++i) {
        if (src[i] >= 97 && src[i] <= 122) // if the char in is the range a-z
        {
            src[i] -= 32; // convert to uppercase
        }
    }
}

/// @param src have to be a zero-terminated string
static void flip_words(char* src){
    int i, j, k;
    size_t size = strlen(src);

    if(size == 0) return;
    j = 0;

    for (i = 0; i <= size; ++i) {
        if(src[i] == ' ' || src[i] == '\0'){
            k = i - 1;
            while(j <= k){
                char tmp = src[j];
                src[j++] = src[k];
                src[k--] = tmp;
            }
            j = i + 1;
        }
    }
}

static int __init string_processor_init(void)
{
    char string_to_modify[64];
    strcpy(string_to_modify, str);
    printk(KERN_INFO MODULE_TAG": Input str: %s\n", string_to_modify);
    to_uppercase(string_to_modify);
    flip_words(string_to_modify);
    printk(KERN_INFO MODULE_TAG": Output str: %s\n", string_to_modify);
    return -1;
}

module_init(string_processor_init)
