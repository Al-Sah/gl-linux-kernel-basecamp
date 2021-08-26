#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xone");
MODULE_VERSION("0.01");
MODULE_DESCRIPTION("Test ex02 Linux module.");

extern char *test_01(void);
extern char *test_02(void);

static int __init test_ex02_init(void)
{
    printk(KERN_INFO "test_ex02: Hello!!!\n");
    printk(KERN_INFO "test_ex02: %s\n", test_01());
    printk(KERN_INFO "test_ex02: %s\n", test_02());
    printk(KERN_INFO "test_ex02: Bye...\n");
    return -1;
}

module_init(test_ex02_init);