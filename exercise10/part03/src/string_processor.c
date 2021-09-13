#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple string processor");
MODULE_VERSION("0.1");


#define MODULE_TAG      "string_processor"
#define PROC_DIRECTORY  "string_processor"
#define BUFFER_SIZE     256
#define SETTINGS_BUFFER_SIZE 8
#define CLASS_ATTR(_name, _mode, _show, _store) struct class_attribute class_attr_##_name = __ATTR(_name, _mode, _show, _store)


static char settings_buffer[ SETTINGS_BUFFER_SIZE ] = "0";
static struct class *string_processor_class;

static char* proc_buffer;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_in_file, *proc_out_file;

static ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
static ssize_t proc_write(__attribute__((unused)) struct file *file_p, const char __user *buffer, size_t length,
                          __attribute__((unused)) loff_t *offset);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_in_fops = {
    .proc_write = proc_write,
};
static const struct proc_ops proc_out_fops = {
    .proc_read  = proc_read,
};
#else
static struct file_operations proc_in_fops = {
        .write = proc_write,
};
static struct file_operations proc_out_fops = {
        .read  = proc_read,
};
#endif

static ssize_t sys_show(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, char *buf );
static ssize_t sys_store(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count );

CLASS_ATTR( settings, ( S_IWUSR | S_IRUGO ), &sys_show, &sys_store );

static int  create_proc_entries(void);
static void delete_proc_entries(void);

static int create_buffer(char **buffer);
static void clean_buffer(char** buffer);

/// @param src have to be a zero-terminated string
static void to_uppercase(char* src);
static void flip_words(char* src);


static int __init string_processor_init(void)
{
    int err;

    err = create_buffer(&proc_buffer);
    if (err) {
        goto error;
    }
    err = create_proc_entries();
    if (err) {
        goto error;
    }

    string_processor_class = class_create(THIS_MODULE, "string_processor_class" );
    if( IS_ERR(string_processor_class) ){
        printk( KERN_WARNING MODULE_TAG": Failed to create sys class" );
    }
    if(class_create_file( string_processor_class, &class_attr_settings ) != 0){
        printk(KERN_WARNING MODULE_TAG ": Failed to create sys class");
    }
    printk(KERN_INFO MODULE_TAG ": Loaded\n");
    return 0;

error:

    printk(KERN_ERR MODULE_TAG ": Failed to load\n");
    delete_proc_entries();
    clean_buffer(&proc_buffer);

    class_remove_file( string_processor_class, &class_attr_settings );
    class_destroy( string_processor_class );
    return err;
}

static void __exit string_processor_exit(void){
    delete_proc_entries();
    clean_buffer(&proc_buffer);

    class_remove_file( string_processor_class, &class_attr_settings );
    class_destroy( string_processor_class );
    printk(KERN_INFO MODULE_TAG ": Exited\n");
}

module_init(string_processor_init)
module_exit(string_processor_exit)



static ssize_t sys_show(__attribute__((unused)) struct class *class,
        __attribute__((unused)) struct class_attribute *attr, char *buf ) {
    strcpy(buf, settings_buffer);
    return (ssize_t)strlen(buf);
}

static ssize_t sys_store(__attribute__((unused)) struct class *class,
        __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count ) {

    switch (buf[0]) {
        case '0': strcpy(settings_buffer, "0"); break; // no modification
        case '1': strcpy(settings_buffer, "1"); break; // flip_words
        case '2': strcpy(settings_buffer, "2"); break; // to_uppercase
        case '3': strcpy(settings_buffer, "3"); break; // flip_words and to_uppercase
        default : printk( KERN_WARNING MODULE_TAG": Failed to update mode");
    }
    settings_buffer[ count ] = '\0';
    return (ssize_t)count;
}


static int create_proc_entries(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }

    proc_in_file = proc_create("input", 0, proc_dir, &proc_in_fops);
    if (proc_in_file == NULL ) {
        return -EFAULT;
    }
    proc_out_file = proc_create("output", 0, proc_dir, &proc_out_fops);
    if (proc_out_file == NULL) {
        return -EFAULT;
    }

    return 0;
}

static void delete_proc_entries(void)
{
    if (proc_in_file)
    {
        remove_proc_entry("input", proc_dir);
        proc_in_file = NULL;
    }
    if (proc_out_file)
    {
        remove_proc_entry("output", proc_dir);
        proc_out_file = NULL;
    }

    if (proc_dir)
    {
        remove_proc_entry(PROC_DIRECTORY, NULL);
        proc_dir = NULL;
    }
}

static ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
{
    char tmp_buffer[BUFFER_SIZE];
    length = strlen(proc_buffer);
    strcpy(tmp_buffer, proc_buffer);

    if( *offset != 0 ) {
        return 0;
    }

    if(settings_buffer[0] == '3' || settings_buffer[0] == '1' ){
        flip_words(tmp_buffer);
    }
    if(settings_buffer[0] == '3' || settings_buffer[0] == '2' ){
        to_uppercase(tmp_buffer);
    }

    if( copy_to_user( buffer, tmp_buffer, length )) {
        printk(KERN_WARNING MODULE_TAG ": Failed to copy some chars");
        return -EINVAL;
    }
    *offset = (loff_t)length;
    return (ssize_t)length;
}


static ssize_t proc_write(__attribute__((unused)) struct file *file_p,
        const char __user *buffer, size_t length, __attribute__((unused)) loff_t *offset)
{
    size_t msg_length, failed;
    if(length > BUFFER_SIZE){
        printk(KERN_WARNING MODULE_TAG "Reduce message length from %lu to %u chars\n", (ulong)length, BUFFER_SIZE);
        msg_length = BUFFER_SIZE;
    }  else{
        msg_length = length;
    }

    failed = copy_from_user(proc_buffer, buffer, msg_length);
    if (failed != 0){
        printk(KERN_WARNING MODULE_TAG ": Failed to copy %lu from %lu \n", (ulong)failed, (ulong)length);
    }
    return (ssize_t)length;
}


static void to_uppercase(char* src){
    int i;
    for (i = 0; i < strlen(src); ++i) {
        if (src[i] >= 97 && src[i] <= 122) // if the char in is the range a-z
        {
            src[i] -= 32; // convert to uppercase
        }
    }
}

static void flip_words(char* src){
    int i, j, k;
    size_t size = strlen(src);

    if(size == 0) return;
    j = 0;

    for (i = 0; i <= size; ++i) {
        if(src[i] == ' ' || src[i] == '\0' || src[i] == '\n' || src[i] == '\r'){
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

static int create_buffer(char **buffer)
{
    *buffer = (char*) kmalloc(BUFFER_SIZE, GFP_KERNEL);
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
}