#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple time manager");
MODULE_VERSION("0.1");


#define MODULE_TAG      "time_manager"
#define PROC_DIRECTORY  "time_manager"

#define generate_str(number, str)                               \
if (number < 10){ snprintf(str, sizeof(str), "0%hd", number);   \
} else { snprintf(str, sizeof(str), "%hd", number);}


unsigned long jiffies_on_last_call = 0;

static char* proc_read_msg  = "Check result in dmesg\n";

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_test_file;

static char* get_time_str(ulong current_jiffies);
static ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_read  = proc_read,
};
#else
static struct file_operations proc_fops = {
        .read  = proc_read,
};
#endif


static int  create_proc_entry(void);
static void delete_proc_entry(void);

/*static int create_buffer(char **buffer);
static void clean_buffer(char** buffer);*/


static int __init string_processor_init(void)
{
    int err;
    do {
        err = create_proc_entry();
        if (err) {
            printk(KERN_WARNING MODULE_TAG": Failed to create proc interface");
            break;
        }
        printk(KERN_INFO MODULE_TAG ": Loaded\n");
        return 0;
    } while (42);

    printk(KERN_ERR MODULE_TAG ": Failed to load\n");
    delete_proc_entry();
    return err;
}

static void __exit string_processor_exit(void){
    delete_proc_entry();
    printk(KERN_INFO MODULE_TAG ": Exited\n");
}

module_init(string_processor_init)
module_exit(string_processor_exit)




static int create_proc_entry(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }

    proc_test_file = proc_create("get_time", 0, proc_dir, &proc_fops);
    if (proc_test_file == NULL ) {
        return -EFAULT;
    }

    return 0;
}

static void delete_proc_entry(void)
{
    if (proc_test_file)
    {
        remove_proc_entry("get_time", proc_dir);
        proc_test_file = NULL;
    }
    if (proc_dir)
    {
        remove_proc_entry(PROC_DIRECTORY, NULL);
        proc_dir = NULL;
    }
}

static ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
{
    length = strlen(proc_read_msg);

    if( *offset != 0 ) {
        return 0;
    }
    *offset = (loff_t)length;

    if( copy_to_user( buffer, proc_read_msg, length )) {
        printk(KERN_WARNING MODULE_TAG ": Failed to copy some chars");
        return -EINVAL;
    }
    if(jiffies_on_last_call == 0){
        printk(KERN_INFO MODULE_TAG ": First time reading test file");
        jiffies_on_last_call = jiffies;
        return (ssize_t)length;
    }

    printk( KERN_INFO " From last read:  %s\n", get_time_str(jiffies - jiffies_on_last_call) );
    jiffies_on_last_call = jiffies;
    return (ssize_t)length;
}


static char* get_time_str(ulong current_jiffies){
    static char res[10];
    size_t _seconds = current_jiffies / HZ;
    short hours, minutes, seconds;
    char hours_str[4], minutes_str[3], seconds_str[3];

    hours = (short)(_seconds / 3600);
    generate_str(hours, hours_str)

    minutes = (short)((_seconds / 60) % 60);
    generate_str(minutes, minutes_str)

    seconds = (short)(_seconds % 60);
    generate_str(seconds, seconds_str)

    snprintf(res, sizeof(res), "%s:%s:%s", hours_str,minutes_str,seconds_str);
    return res;
}
/*
static int create_buffer(char **buffer)
{
    *buffer = (char*) kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (*buffer == NULL){
        printk(KERN_WARNING MODULE_TAG": Failed to create buffer");
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
}*/
