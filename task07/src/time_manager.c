#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/slab.h>
//#include <linux/time.h>
#include <linux/fs.h>
#include <linux/ktime.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple time manager");
MODULE_VERSION("0.1");


#define MODULE_TAG      "time_manager"
#define PROC_DIRECTORY  "time_manager"

#define generate_str(number, str)                               \
if ((number) < 10){ snprintf(str, sizeof(str), "0%hd", number);   \
} else { snprintf(str, sizeof(str), "%hd", number);}

unsigned long jiffies_on_last_call = 0;

static char* proc_read_msg  = "Check result in dmesg\n";

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_test_file, *proc_absolute_time;

static char* get_time_str(size_t seconds_to_convert);
static ssize_t test_file_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
static ssize_t get_absolute_time(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops proc_test_fops = {
    .proc_read  = test_file_read,
};
static struct proc_ops proc_absolute_time_fops = {
        .proc_read  = get_absolute_time,
};
#else
static struct file_operations proc_test_fops = {
        .read  = test_file_read,
};
static struct file_operations proc_absolute_time_fops = {
        .read  = get_absolute_time,
};
#endif


static int  create_proc_entry(void);
static void delete_proc_entry(void);

/*static int create_buffer(char **buffer);
static void clean_buffer(char** buffer);
struct timer_list my_timer;
static void timer_function(struct timer_list *data){
    printk("Time up");
    // modify the timer for next time
    mod_timer(&my_timer, jiffies + HZ / 2);
}*/


static int __init time_manager_init(void)
{
    int err;
    do {
        err = create_proc_entry();
        if (err) {
            printk(KERN_WARNING MODULE_TAG": Failed to create proc interface");
            break;
        }
        /*init_timer_key(&my_timer, timer_function, 0, NULL, NULL);
        my_timer.expires = jiffies + HZ ;
        my_timer.function = timer_function;
        add_timer(&my_timer);*/

        printk(KERN_INFO MODULE_TAG ": Loaded\n");
        return 0;
    } while (42);

    printk(KERN_ERR MODULE_TAG ": Failed to load\n");
    delete_proc_entry();
    return err;
}

static void __exit time_manager_exit(void){
    delete_proc_entry();
    //del_timer(&my_timer);
    printk(KERN_INFO MODULE_TAG ": Exited\n");
}

module_init(time_manager_init)
module_exit(time_manager_exit)




static int create_proc_entry(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }

    proc_test_file = proc_create("last_read_time", 0, proc_dir, &proc_test_fops);
    if (proc_test_file == NULL ) {
        return -EFAULT;
    }
    proc_absolute_time = proc_create("absolute_time", 0, proc_dir, &proc_absolute_time_fops);
    if (proc_absolute_time == NULL ) {
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

static ssize_t test_file_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
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

static ssize_t get_absolute_time(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset){
    char  boot_time[10], res[60]; // realtime[10],
    //struct timespec timespec_now;


    if( *offset != 0 ) {
        return 0;
    }
    *offset = (loff_t)length;

    //getnstimeofday(&timespec_now);
    strcpy(boot_time, get_time_str(jiffies / HZ));
    //strcpy(realtime, get_time_str(timespec_now.tv_sec));

    snprintf(res, sizeof(res), "Time from boot: %s\n", boot_time);
    length = strlen(res);


    if( copy_to_user( buffer, res, length )) {
        printk(KERN_WARNING MODULE_TAG ": Failed to copy some chars");
        return -EINVAL;
    }
    return (ssize_t)length;
}


static char* get_time_str(size_t seconds_to_convert){
    static char res[10];
    short hours, minutes, seconds;
    char hours_str[4], minutes_str[3], seconds_str[3];

    hours = (short)(seconds_to_convert / 3600);
    generate_str(hours, hours_str)

    minutes = (short)((seconds_to_convert / 60) % 60);
    generate_str(minutes, minutes_str)

    seconds = (short)(seconds_to_convert % 60);
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
