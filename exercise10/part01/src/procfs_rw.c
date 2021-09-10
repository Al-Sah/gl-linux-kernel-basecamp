#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>


static ssize_t example_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
static ssize_t example_write(struct file *file_p, const char __user *buffer, size_t length, loff_t *offset);


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Example for procfs read/write");
MODULE_VERSION("0.1");


#define MODULE_TAG      "example_module "
#define PROC_DIRECTORY  "example"
#define PROC_FILENAME   "buffer"
#define BUFFER_SIZE     10


static char* proc_buffer;
static size_t proc_msg_length;
static size_t proc_msg_read_pos;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_read  = example_read,
    .proc_write = example_write,
};
#else
static struct file_operations proc_fops = {
    .read  = example_read,
    .write = example_write,
};
#endif


static int create_buffer(void)
{
    proc_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (NULL == proc_buffer)
        return -ENOMEM;
    proc_msg_length = 0;

    return 0;
}


static void cleanup_buffer(void)
{
    if (proc_buffer) {
        kfree(proc_buffer);
        proc_buffer = NULL;
    }
    proc_msg_length = 0;
}


static int create_proc_example(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (NULL == proc_dir)
        return -EFAULT;

    proc_file = proc_create(PROC_FILENAME, 0, proc_dir, &proc_fops);
    if (NULL == proc_file)
        return -EFAULT;

    return 0;
}


static void cleanup_proc_example(void)
{
    if (proc_file)
    {
        remove_proc_entry(PROC_FILENAME, proc_dir);
        proc_file = NULL;
    }
    if (proc_dir)
    {
        remove_proc_entry(PROC_DIRECTORY, NULL);
        proc_dir = NULL;
    }
}


static ssize_t example_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
{
    size_t left;

    if (length > (proc_msg_length - proc_msg_read_pos)) {
        length = (proc_msg_length - proc_msg_read_pos);
    }

    left = copy_to_user(buffer, &proc_buffer[proc_msg_read_pos], length);

    proc_msg_read_pos += length - left;

    if (left){
        printk(KERN_ERR MODULE_TAG "failed to read %lu from %lu chars\n", left, length);
    } else {
        printk(KERN_NOTICE MODULE_TAG "read %lu chars\n", length);
    }

    return (ssize_t)(length - left);
}


static ssize_t example_write(struct file *file_p, const char __user *buffer, size_t length, loff_t *offset)
{
    size_t msg_length;
    size_t left;

    if (length > BUFFER_SIZE)
    {
        printk(KERN_WARNING MODULE_TAG "Reduce message length from %lu to %u chars\n", length, BUFFER_SIZE);
        msg_length = BUFFER_SIZE;
    }
    else{
        msg_length = length;
    }

    left = copy_from_user(proc_buffer, buffer, msg_length);

    proc_msg_length = msg_length - left;
    proc_msg_read_pos = 0;

    if (left){
        printk(KERN_ERR MODULE_TAG "failed to write %lu from %lu chars\n", left, msg_length);
    }
    else{
        printk(KERN_NOTICE MODULE_TAG "written %lu chars\n", msg_length);
    }

    return (ssize_t)length;
}


static int __init example_init(void)
{
    int err;

    err = create_buffer();
    if (err)
        goto error;

    err = create_proc_example();
    if (err)
        goto error;

    printk(KERN_NOTICE MODULE_TAG "loaded\n");
    return 0;

error:
    printk(KERN_ERR MODULE_TAG "failed to load\n");
    cleanup_proc_example();
    cleanup_buffer();
    return err;
}


static void __exit example_exit(void)
{
    cleanup_proc_example();
    cleanup_buffer();
    printk(KERN_NOTICE MODULE_TAG "exited\n");
}


module_init(example_init)
module_exit(example_exit)
