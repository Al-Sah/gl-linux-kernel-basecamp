#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/fs.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple time manager");
MODULE_VERSION("0.1");

#define MODULE_TAG      "time_manager"
#define PROC_DIRECTORY  "time_manager"
#define SETTINGS_BUFFER_SIZE 8

#define EACH_SECONDS 5

#define NS_IN_SEC 1000000000
#define NS_IN_MS 1000000
#define NS_IN_US 1000


#define CLASS_ATTR(_name, _mode, _show, _store) struct class_attribute class_attr_##_name = __ATTR(_name, _mode, _show, _store)
#define generate_str(number, str)                               \
if ((number) < 10){ snprintf(str, sizeof(str), "0%d", number);   \
} else { snprintf(str, sizeof(str), "%d", number);}
static char settings_buffer[ SETTINGS_BUFFER_SIZE ] = "0";

short full_mode = 0;
static ssize_t sys_show(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, char *buf );
static ssize_t sys_store(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count );

CLASS_ATTR( settings, ( S_IWUSR | S_IRUGO ), &sys_show, &sys_store );

#ifdef CONFIG_X86_64
static char* get_time_str_ns(u64 src);
#endif

static struct class *time_manager;
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

struct timer_list my_timer;
static void timer_function(struct timer_list *data){
    printk(KERN_INFO MODULE_TAG": Random value: %d", get_random_int());
    mod_timer(&my_timer, jiffies + EACH_SECONDS * HZ);
}


static int __init time_manager_init(void)
{
    int err;
    do {
        err = create_proc_entry();
        if (err) {
            printk(KERN_WARNING MODULE_TAG": Failed to create proc interface");
            break;
        }
        init_timer_key(&my_timer, timer_function, 0, NULL, NULL);
        my_timer.expires = jiffies + EACH_SECONDS * HZ;
        my_timer.function = timer_function;
        add_timer(&my_timer);

        time_manager = class_create(THIS_MODULE, "time_manager");
        if (IS_ERR(time_manager)) {
            printk(KERN_WARNING MODULE_TAG": Failed to create sys class");
            break;
        }
        if (class_create_file(time_manager, &class_attr_settings) != 0) {
            printk(KERN_WARNING MODULE_TAG ": Failed to create sys_fs 'settings' file");
            break;
        }

        printk(KERN_INFO MODULE_TAG ": Loaded\n");
        return 0;
    } while (42);

    printk(KERN_ERR MODULE_TAG ": Failed to load\n");
    delete_proc_entry();
    return err;
}

static void __exit time_manager_exit(void){
    delete_proc_entry();
    class_remove_file( time_manager, &class_attr_settings );
    class_destroy( time_manager );
    del_timer(&my_timer);
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
        remove_proc_entry("last_read_time", proc_dir);
        proc_test_file = NULL;
    }
    if (proc_absolute_time)
    {
        remove_proc_entry("absolute_time", proc_dir);
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
    if(full_mode){
        printk( KERN_INFO " From last read:  %s\n", get_time_str((jiffies - jiffies_on_last_call) / HZ) );
    } else{
        printk( KERN_INFO " From last read:  %lu\n", (ulong)(jiffies - jiffies_on_last_call) / HZ);
    }

    jiffies_on_last_call = jiffies;
    return (ssize_t)length;
}

static ssize_t get_absolute_time(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset){
    char  boot_time[22], real_time[22], mono_time[22], res[512];
    u64 real, boot, mono;
#ifdef CONFIG_X86_32
    struct timespec64 ts;
#endif
    if( *offset != 0 ) {
        return 0;
    }
    *offset = (loff_t)length;

    mono = ktime_get_mono_fast_ns();
    boot = ktime_get_boot_fast_ns();
    real = ktime_get_real_fast_ns();

#ifdef CONFIG_X86_32
    ts = ktime_to_timespec64(real);
    strcpy(real_time, get_time_str(ts.tv_sec));

    ts = ktime_to_timespec64(boot);
    strcpy(boot_time, get_time_str(ts.tv_sec));

    ts = ktime_to_timespec64(mono);
    strcpy(mono_time, get_time_str(ts.tv_sec));
#else
    strcpy(real_time, get_time_str_ns(real));
    strcpy(boot_time, get_time_str_ns(boot));
    strcpy(mono_time, get_time_str_ns(mono));
#endif
    snprintf(res, sizeof(res), " boot_time: %s\n real_time: %s\n mono_time: %s\n", boot_time, real_time, mono_time);
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

    hours = (short)((seconds_to_convert / 3600)  % 24);
    generate_str(hours, hours_str)

    minutes = (short)((seconds_to_convert / 60) % 60);
    generate_str(minutes, minutes_str)

    seconds = (short)(seconds_to_convert % 60);
    generate_str(seconds, seconds_str)

    snprintf(res, sizeof(res), "%s:%s:%s", hours_str,minutes_str,seconds_str);
    return res;
}

#ifdef CONFIG_X86_64
static char* get_time_str_ns(u64 src){
    static char res[22];
    char hours[3], min[3], sec[3];

    ushort ms, us, ns;
    generate_str((short)(((src / 3600) / NS_IN_SEC) % 24), hours)
    generate_str((short)((src / 60 / NS_IN_SEC) % 60), min)
    generate_str((short)((src / NS_IN_SEC) % 60), sec)

    ns = (ushort)src % 1000;
    us = (ushort)(src / NS_IN_US) % 1000;
    ms = (ushort)(src / NS_IN_MS) % 1000;

    snprintf(res, sizeof(res), "%s:%s:%s %d:%d:%d", hours, min, sec, ms, us, ns);
    return res;
}
#endif


static ssize_t sys_show(__attribute__((unused)) struct class *class,
                        __attribute__((unused)) struct class_attribute *attr, char *buf ) {
    strcpy(buf, settings_buffer);
    return (ssize_t)strlen(buf);
}

static ssize_t sys_store(__attribute__((unused)) struct class *class,
                         __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count ) {
    switch (buf[0]) {
        case '0':
            strcpy(settings_buffer, "0\0");
            full_mode = 0;
            break;
        case '1':
            strcpy(settings_buffer, "1\0");
            full_mode = 1;
            break;
        default :
            printk(KERN_WARNING MODULE_TAG": Failed to update full_mode");
    }
    return (ssize_t) count;
}