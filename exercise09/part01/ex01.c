#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("alx");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

static int __init ex01_init(void)
{
    printk(KERN_INFO "Hello!!!\n");
    return 0;
}

static void __exit ex01_exit(void)
{
    printk(KERN_INFO "Bye...\n");
}

module_init(ex01_init);
module_exit(ex01_exit);
