#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple string processor");
MODULE_VERSION("0.1");


#define MODULE_TAG      "string_processor"
#define PROC_DIRECTORY  "string_processor"
#define PROC_FILENAME   "buffer"
#define BUFFER_SIZE     256


static char* proc_buffer;
static bool read;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;

static ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
static ssize_t proc_write(struct file *file_p, const char __user *buffer, size_t length, loff_t *offset);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_fops = {
    .proc_read  = proc_read,
    .proc_write = proc_write,
};
#else
static struct file_operations proc_fops = {
        .read  = proc_read,
        .write = proc_write,
};
#endif


static int  create_proc_entries(void);
static void delete_proc_entries(void);

static int create_buffer(char ** buffer, int size);
static void clean_buffer(char** buffer);

/// @param src have to be a zero-terminated string
static void to_uppercase(char* src);
static void flip_words(char* src);


static int __init string_processor_init(void)
{
    int err;

    err = create_buffer(&proc_buffer, BUFFER_SIZE);
    if (err) {
        goto error;
    }
    err = create_proc_entries();
    if (err) {
        goto error;
    }

    printk(KERN_INFO MODULE_TAG ": Loaded\n");
    return 0;

error:

    printk(KERN_ERR MODULE_TAG ": Failed to load\n");
    delete_proc_entries();
    clean_buffer(&proc_buffer);
    return err;
}

static void __exit string_processor_exit(void){
    delete_proc_entries();
    clean_buffer(&proc_buffer);
    printk(KERN_INFO MODULE_TAG ": Exited\n");
}

module_init(string_processor_init)
module_exit(string_processor_exit)



static int create_proc_entries(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (NULL == proc_dir)
        return -EFAULT;

    proc_file = proc_create(PROC_FILENAME, 0, proc_dir, &proc_fops);
    if (NULL == proc_file)
        return -EFAULT;

    return 0;
}

static void delete_proc_entries(void)
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

static ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
{
    char tmp_buffer[BUFFER_SIZE];
    length = strlen(proc_buffer);
    strcpy(tmp_buffer, proc_buffer);

    if( *offset != 0 ) {
        return 0;
    }

    flip_words(tmp_buffer);
    to_uppercase(tmp_buffer);

    if( copy_to_user( buffer, tmp_buffer, length )) {
        printk(KERN_WARNING MODULE_TAG ": Failed to copy some chars");
        return -EINVAL;
    }
    *offset = (loff_t)length;
    return (ssize_t)length;
}


static ssize_t proc_write(struct file *file_p, const char __user *buffer, size_t length, loff_t *offset)
{
    size_t msg_length, failed;
    if(length > BUFFER_SIZE){
        printk(KERN_WARNING MODULE_TAG "Reduce message length from %lu to %u chars\n", length, BUFFER_SIZE);
        msg_length = BUFFER_SIZE;
    }  else{
        msg_length = length;
    }

    failed = copy_from_user(proc_buffer, buffer, msg_length);
    if (failed != 0){
        printk(KERN_WARNING MODULE_TAG ": Failed to copy %lu from %lu \n", failed, length);
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
}