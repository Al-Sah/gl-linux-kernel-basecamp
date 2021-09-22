#include <linux/module.h>
#include <linux/kernel.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple memory testing module");
MODULE_VERSION("0.1");

#define MODULE_TAG "memory_tester"

static int __init memory_tester_init(void)
{
    printk(KERN_NOTICE MODULE_TAG "loaded\n");
    return 0;
}


static void __exit memory_tester_exit(void)
{
    printk(KERN_NOTICE MODULE_TAG "exited\n");
}


module_init(memory_tester_init)
module_exit(memory_tester_exit)
