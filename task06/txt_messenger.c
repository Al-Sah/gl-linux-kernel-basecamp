#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>


MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Al_Sah" );
MODULE_VERSION( "0.1" );

#define MODNAME "txt_messenger"

static int major = 0;
static int dev_buffer_max_size = 1024;
//static int buffer_size = 0;
module_param(dev_buffer_max_size, int, S_IRUGO);


static char* messenger_buffer;


static struct cdev hcdev;
static struct class *devclass;


static int create_buffer(char ** buffer, int size)
{
    *buffer = (char*) kmalloc(size, GFP_KERNEL);
    if (*buffer == NULL){
        return -1;
    }
    return 0;
}

static void clean_buffer(char** buffer)
{
    if (*buffer) {
        kfree(*buffer);
        *buffer = NULL;
    }
    //buffer_size = 0;
}


static ssize_t dev_read( struct file * file, char * buf, size_t count, loff_t *ppos ) {

    u32 len = (u32)strlen(messenger_buffer);

    if( count < len ) {
        printk( KERN_INFO "=== dev_read messenger buffer size > count: %u > %ld\n", len, (long)count );
        return -EINVAL;
    }

    if( *ppos != 0 ) {
        printk( KERN_INFO "=== read return : 0\n" );  // EOF
        return 0;
    }

    if( copy_to_user( buf, messenger_buffer, len ) ) return -EINVAL;
    *ppos = len;

    printk( KERN_INFO "=== read return : %d\n", len );
    return len;
}


static ssize_t dev_write(struct file *file_p, const char __user *buffer, size_t length, loff_t *offset)
{
    size_t msg_length;
    size_t left;

    if (length > dev_buffer_max_size) {
        printk(KERN_WARNING " Reduce message length from %lu to %u chars\n", length, dev_buffer_max_size);
        printk(KERN_WARNING " Buffer have to be cleaned ");
        msg_length = dev_buffer_max_size;
    }
    else{
        msg_length = length;
    }

    left = copy_from_user(messenger_buffer, buffer, msg_length);


    if (left)
        printk(KERN_ERR "failed to write %lu from %lu chars\n", left, msg_length);
    else {
        printk(KERN_NOTICE "written %lu chars\n", msg_length);
    }

    return (ssize_t)length;
}

static const struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .read  = dev_read,
    .write = dev_write,
};


static int __init dev_init( void ) {
    int ret;
    dev_t dev;

    // Errors check
    ret = alloc_chrdev_region(&dev, 0, 1, MODNAME );
    if( ret < 0 ) {
        printk(KERN_ERR "=== Can not register char device region\n" );
        goto err;
    }
    major = MAJOR(dev);

    cdev_init( &hcdev, &dev_fops );
    hcdev.owner = THIS_MODULE;
    ret = cdev_add(&hcdev, dev, 1);

    if(ret < 0 ) {
        unregister_chrdev_region(MKDEV(major, 0 ), 1);
        printk( KERN_ERR "=== Can not add char device\n" );
        goto err;
    }

    devclass = class_create( THIS_MODULE, "messenger_class");
    device_create(devclass, NULL, dev, NULL, "messenger_buffer");


    ret = create_buffer(&messenger_buffer, dev_buffer_max_size);
    //ret = create_buffer();
    if (ret){
        goto err;
    }

    printk(KERN_INFO "======== module installed %d:[%d-%d] ===========\n", MAJOR(dev), 0, MINOR(dev));
    printk(KERN_INFO " Buffer size: %d\n", dev_buffer_max_size);

err:
    return ret;
}

static void __exit dev_exit( void ) {

    device_destroy(devclass, MKDEV(major, 0));
    class_destroy( devclass );
    cdev_del( &hcdev );
    unregister_chrdev_region(MKDEV(major, 0 ), 1 );

    clean_buffer(&messenger_buffer);

    printk( KERN_INFO "=============== module removed ==================\n" );
}


module_init(dev_init)
module_exit(dev_exit)