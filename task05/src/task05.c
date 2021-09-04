
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple currency convertor");
MODULE_VERSION("0.1");

static int __init task05_init(void)
{
    printk(KERN_INFO "task05: Hello...\n");
    return 0;
}


static void __exit task05_exit(void)
{

    printk(KERN_INFO "task05: Bye...\n");
}

module_init(task05_init);
module_exit(task05_exit);