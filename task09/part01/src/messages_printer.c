#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Module which output specified messages using magic");
MODULE_VERSION("0.1");

#define CLASS_ATTR(_name, _mode, _show, _store) struct class_attribute class_attr_##_name = __ATTR(_name, _mode, _show, _store)
#define MODULE_TAG      "messages_printer"
#define PROC_DIRECTORY  "messages_printer"
#define BUFFER_SIZE     256

static char* proc_buffer = NULL;
static char* sys_buffer = NULL;
static struct proc_dir_entry *proc_dir, *proc_data_file;

static ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
static ssize_t proc_write(__attribute__((unused)) struct file *file_p, const char __user *buffer, size_t length,
                          __attribute__((unused)) loff_t *offset);

static int create_buffer(char **buffer, size_t size);
static void clean_buffer(char **buffer);

static int  create_proc_entries(void);
static void delete_proc_entries(void);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_write = proc_write,
    //.proc_read  = proc_read,
};
#else
static struct file_operations proc_fops = {
        .write = proc_write,
        //.read  = proc_read,
};
#endif

static struct class *messages_printer;
static ssize_t sys_show(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, char *buf );
static ssize_t sys_store(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count );
CLASS_ATTR( settings, ( S_IWUSR | S_IRUGO ), &sys_show, &sys_store );

long message_delay = 20;
long output_time = 100;

static void inline on_exit(void){
    delete_proc_entries();
    clean_buffer(&sys_buffer);
    clean_buffer(&proc_buffer);

    class_remove_file( messages_printer, &class_attr_settings );
    class_destroy( messages_printer );
}

static int __init messages_printer_init(void)
{
    int err;
    do {
        err = create_buffer(&proc_buffer, BUFFER_SIZE);
        if (err) {
            break;
        }
        err = create_buffer(&sys_buffer, BUFFER_SIZE);
        if (err) {
            break;
        }
        err = create_proc_entries();
        if (err) {
            printk(KERN_WARNING MODULE_TAG": Failed to create proc interface");
            break;
        }
        messages_printer = class_create(THIS_MODULE, "string_processor_class");
        if (IS_ERR(messages_printer)) {
            printk(KERN_WARNING MODULE_TAG": Failed to create sys class");
            break;
        }
        if (class_create_file(messages_printer, &class_attr_settings) != 0) {
            printk(KERN_WARNING MODULE_TAG ": Failed to create sys_fs 'settings' file");
            break;
        }
        printk(KERN_INFO MODULE_TAG ": Loaded\n");
        return 0;
    } while (42);

    printk(KERN_ERR MODULE_TAG ": Failed to load\n");
    on_exit();
    return err;
}

static void __exit messages_printer_exit(void){
    on_exit();
    printk(KERN_INFO MODULE_TAG ": Exited\n");
}

module_init(messages_printer_init)
module_exit(messages_printer_exit)


static ssize_t proc_write(__attribute__((unused)) struct file *file_p,
                          const char __user *buffer, size_t length, __attribute__((unused)) loff_t *offset)
{
    size_t msg_length, failed;
    if(length+1 > BUFFER_SIZE){
        printk(KERN_WARNING MODULE_TAG "Reduce message length from %lu to %u chars\n", (ulong)length, BUFFER_SIZE);
        msg_length = BUFFER_SIZE-1;
    }  else{
        msg_length = length;
    }

    failed = copy_from_user(proc_buffer, buffer, msg_length);
    proc_buffer[msg_length]='\0';

    if (failed != 0){
        printk(KERN_WARNING MODULE_TAG ": Failed to copy %lu from %lu \n", (ulong)failed, (ulong)length);
    }
    printk(KERN_INFO MODULE_TAG ": Message: '%s', %ld, %ld\n", proc_buffer, message_delay, output_time);
    return (ssize_t)length;
}


static ssize_t sys_show(__attribute__((unused)) struct class *class,
                        __attribute__((unused)) struct class_attribute *attr, char *buf ) {
    strcpy(buf, sys_buffer);
    return (ssize_t)strlen(buf);
}

static ssize_t sys_store(__attribute__((unused)) struct class *class,
                         __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count ) {

    char *tmp_buffer = NULL;
    char *token = NULL, *substr = NULL;
    long from_str;
    size_t i;

    create_buffer(&tmp_buffer, count);
    strncpy(tmp_buffer, buf, count);
    if(count > BUFFER_SIZE){
        printk(KERN_WARNING MODULE_TAG ": Input buf too long");
        return (ssize_t)count;
    }
    strncpy(sys_buffer, buf, count);
    sys_buffer[count]='\0';

    token = tmp_buffer;
    for(i = 0; i < 2; ++i){
        substr = strsep(&token, " ");
        if(substr == NULL){
            break;
        }
        if(kstrtol(substr, 10, &from_str) == 0){
            if(i == 0){
                message_delay = from_str;
            } else {
                output_time = from_str;
            }
        }
    }
    return (ssize_t)count;
}



static int create_proc_entries(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }
    proc_data_file = proc_create("data", 0, proc_dir, &proc_fops);
    if (proc_data_file == NULL ) {
        return -EFAULT;
    }
    return 0;
}
static void delete_proc_entries(void)
{
    if (proc_data_file){
        remove_proc_entry("data", proc_dir);
        proc_data_file = NULL;
    }
    if (proc_dir) {
        remove_proc_entry(PROC_DIRECTORY, NULL);
        proc_dir = NULL;
    }
}


static int create_buffer(char **buffer, size_t size)
{
    *buffer = (char*) kmalloc(size, GFP_KERNEL);
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
}

