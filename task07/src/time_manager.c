#include "../time_manager.h"


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops proc_fops = {
    .proc_read  = proc_read,
};
static struct proc_ops proc_time_controller_fops = {
        .proc_read  = proc_time_controller_read,
        .proc_write = proc_time_controller_write,
};
#else
static struct file_operations proc_fops = {
        .read  = proc_read,
};
static struct file_operations proc_time_controller_fops = {
        .read  = proc_time_controller_read,
        .write = proc_time_controller_write,
};
#endif

CLASS_ATTR( settings, ( S_IWUSR | S_IRUGO ), &sys_show, &sys_store );

static struct class *time_manager;
static char settings_buffer[ SETTINGS_BUFFER_SIZE ] = "0";

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_last_access, *proc_timestamps, *proc_time_controller;
static struct timer_list my_timer;

static short full_mode     = 0;
static ulong last_jiffies  = 0;


static const char* proc_read_msg   = "To set new real time write it in this file in hh:mm:ss form\n";
static const char* time_access_msg = "have passed since last file access";


// 'private' functions
static inline char *get_timestamps(void);
static inline char *get_last_time_access_str(void);
static inline void run_timer(void);
static inline void on_exit(void);
static time64_t get_sec_from_str(char* buffer);


int __init time_manager_init(void)
{
    int err;
    do {
        err = create_proc_entries();
        if (err) {
            printk(KERN_WARNING MODULE_TAG_D"Failed to create proc interface");
            break;
        }
        err = create_sys_entries();
        if (err) {
            break;
        }
        run_timer();
        printk(KERN_INFO MODULE_TAG_D"***** Loaded *****");
        return 0;
    } while (42);
    printk(KERN_ERR MODULE_TAG_D"***** Failed to load *****");
    on_exit();
    return err;
}

void __exit time_manager_exit(void){
    on_exit();
    printk(KERN_INFO MODULE_TAG_D " ***** Exited ******");
}

module_init(time_manager_init)
module_exit(time_manager_exit)


static inline void run_timer(){
    init_timer_key(&my_timer, timer_callback, 0, NULL, NULL);
    my_timer.expires = jiffies + EACH_SECONDS * HZ;
    my_timer.function = timer_callback;
    add_timer(&my_timer);
}

void timer_callback(__attribute__((unused)) struct timer_list *data){
    printk(KERN_INFO MODULE_TAG_D"timer: generated random value: %d", get_random_int());
    mod_timer(&my_timer, jiffies + EACH_SECONDS * HZ);
}


inline int create_sys_entries(void){
    time_manager = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(time_manager)) {
        printk(KERN_WARNING MODULE_TAG_D" sys_fs: failed to create "CLASS_NAME);
        return -EFAULT;
    }
    if (class_create_file(time_manager, &class_attr_settings) != 0) {
        printk(KERN_WARNING MODULE_TAG_D "sys_fs: failed to create 'settings' file");
        return -EFAULT;
    }
    return 0;
}


static inline void on_exit(){
    remove_proc_entries();
    class_remove_file( time_manager, &class_attr_settings );
    class_destroy( time_manager );
    del_timer(&my_timer);
}


int create_proc_entries(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }

    proc_last_access = proc_create(PROC_LAST_ACCESS_FILE, 0, proc_dir, &proc_fops);
    if (proc_last_access == NULL ) {
        return -EFAULT;
    }

    proc_time_controller = proc_create(PROC_SET_TIME_FILE, 0, proc_dir, &proc_time_controller_fops);
    if (proc_time_controller == NULL ) {
        return -EFAULT;
    }

    proc_timestamps = proc_create(PROC_TIMESTAMPS_FILE, 0, proc_dir, &proc_fops);
    if (proc_timestamps == NULL ) {
        return -EFAULT;
    }
    return 0;
}

void remove_proc_entries(void)
{
    if (proc_last_access != NULL)
    {
        remove_proc_entry(PROC_LAST_ACCESS_FILE, proc_dir);
        proc_last_access = NULL;
    }

    if (proc_timestamps != NULL)
    {
        remove_proc_entry(PROC_TIMESTAMPS_FILE, proc_dir);
        proc_last_access = NULL;
    }

    if (proc_time_controller != NULL)
    {
        remove_proc_entry(PROC_SET_TIME_FILE, proc_dir);
        proc_last_access = NULL;
    }

    if (proc_dir != NULL)
    {
        remove_proc_entry(PROC_DIRECTORY, NULL);
        proc_dir = NULL;
    }
}


ssize_t proc_time_controller_read(__attribute__((unused)) struct file *file_p,
                                  char __user *buffer, size_t length, loff_t *offset){
    size_t failed;
    if( *offset != 0 ) {
        return 0;
    }
    length = strlen(proc_read_msg);
    printk(KERN_INFO MODULE_TAG_D "time_ctrl_read: length %lu", (ulong)length);

    failed = copy_to_user(buffer, proc_read_msg, length );
    if( failed ) {
        printk(KERN_WARNING MODULE_TAG_D "time_ctrl_read: failed to copy %d chars", (int)(failed));
    return -EINVAL;
    }
    *offset = (loff_t)length;
    return (ssize_t)length;
}

ssize_t proc_time_controller_write(__attribute__((unused)) struct file * file_p,
                                   const char __user *buffer, size_t length,
                                   __attribute__((unused)) loff_t *offset){

    static char time_controller_buffer[TIME_STR_SIZE];
    struct timespec64 ts = { .tv_sec = 0, .tv_nsec = 0,};
    size_t failed;

    if(length > TIME_STR_SIZE || length < (TIME_STR_SIZE - 2)) {
        printk(KERN_WARNING MODULE_TAG_D "time_set: wrong time format (bad size) %lu", (ulong)length);
        return (ssize_t) length;
    }

    failed = copy_from_user(time_controller_buffer, buffer, length);
    if (failed != 0){
        printk(KERN_WARNING MODULE_TAG_D "Failed to copy %lu from %lu \n", (ulong)failed, (ulong)length);
    }
    if(time_controller_buffer[2] != ':' || time_controller_buffer[5] != ':' ){
        printk(KERN_WARNING MODULE_TAG_D "time_set: wrong time format (no delimiters)");
        return (ssize_t) length;
    }

    ts.tv_sec = get_sec_from_str(time_controller_buffer);
    if(ts.tv_sec == -1){
        printk(KERN_WARNING MODULE_TAG_D "time_set: parsing failed");
    } else {
        do_settimeofday64(&ts);
    }

    return (ssize_t)length;
}

static time64_t get_sec_from_str(char* buffer){
    char *token = buffer, *substr = NULL;
    long from_str, to_seconds_converter = 3600;
    time64_t result = 0;

    while((substr = strsep(&token, ":")) != NULL ) {
        if(kstrtol(substr, 10, &from_str) == 0){
            result += from_str * to_seconds_converter;
            to_seconds_converter /= 60;
        }else{
            return -1;
        }
    }
    return result;
}


ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset){
    char result[256];
    size_t failed;

    if( *offset != 0 ) {
        return 0;
    }
    *offset = (loff_t)length;

    if(strcmp(file_p->f_path.dentry->d_iname, PROC_TIMESTAMPS_FILE) == 0){
        strcpy(result, get_timestamps());
    } else{
        if(last_jiffies == 0){
            strcpy(result, "First time accessing file\n");
        } else{
            strcpy(result, get_last_time_access_str());
        }
        last_jiffies = jiffies;
    }

    length = strlen(result);
    failed = copy_to_user(buffer, result, length );
    if( failed ) {
        printk(KERN_WARNING MODULE_TAG_D "proc_read: failed to copy %d chars", (int)(failed));
        return -EINVAL;
    }
    return (ssize_t)length;
}


static inline char *get_last_time_access_str(){
    static char result[TIME_STR_SIZE + 50]; // ~50 -> time_access_msg length

    if(full_mode){
        snprintf(result, sizeof(result), " %s %s\n", get_time_str((jiffies - last_jiffies) / HZ), time_access_msg);
    } else{
        snprintf(result, sizeof(result), " %lu seconds %s\n", (ulong)((jiffies - last_jiffies) / HZ), time_access_msg);
    }
    return result;
}


static inline char *get_timestamps(){
    static char result[TIME_STR_LONG_SIZE * 3 + 15 * 3];
    char real_str[TIME_STR_LONG_SIZE], boot_str[TIME_STR_LONG_SIZE], mono_str[TIME_STR_LONG_SIZE];
    u64 real = ktime_get_real_fast_ns(),
        boot = ktime_get_boot_fast_ns(),
        mono = ktime_get_mono_fast_ns();

#ifdef CONFIG_X86_32
    strcpy(real_str, get_time_str(ktime_to_timespec64(real).tv_sec));
    strcpy(boot_str, get_time_str(ktime_to_timespec64(boot).tv_sec));
    strcpy(mono_str, get_time_str(ktime_to_timespec64(mono).tv_sec));
#else
    strcpy(real_str, get_time_str_ns(real));
    strcpy(boot_str, get_time_str_ns(boot));
    strcpy(mono_str, get_time_str_ns(mono));
#endif
    snprintf(result, sizeof(result),
             " boot_time: %s\n real_time: %s\n mono_time: %s\n",
             boot_str, real_str, mono_str);
    return result;
}


char* get_time_str(size_t sec){
    static char res[TIME_STR_SIZE];
    snprintf(res, sizeof(res), "%02d:%02d:%02d",
             (short)((sec / 3600)  % 24), // Extracting hours
             (short)((sec / 60) % 60),    // Extracting minutes
             (short)(sec % 60));          // Extracting seconds
    return res;
}

#ifdef CONFIG_X86_64
char* get_time_str_ns(u64 ns){
    static char res[TIME_STR_LONG_SIZE];
    snprintf(res, sizeof(res), "%02d:%02d:%02d %04d:%04d:%04d",
             (short)(((ns / 3600) / NS_IN_SEC) % 24), // Extracting hours
             (short)((ns / 60 / NS_IN_SEC) % 60),     // Extracting minutes
             (short)((ns / NS_IN_SEC) % 60),          // Extracting seconds

             (short)((ns / NS_IN_MS) % 1000),  // Extracting milliseconds
             (short)((ns / NS_IN_US) % 1000),  // Extracting microseconds
             (short)(ns % 1000));              // Extracting nanoseconds
    return res;
}
#endif


ssize_t sys_show(__attribute__((unused)) struct class *class,
                 __attribute__((unused)) struct class_attribute *attr, char *buf){

    strcpy(buf, settings_buffer);
    return (ssize_t)strlen(buf);
}

ssize_t sys_store(__attribute__((unused)) struct class *class,
                  __attribute__((unused)) struct class_attribute *attr,
                  const char *buf, size_t count ) {

    switch (buf[0]) {
        case SHORT_MODE:
            strcpy(settings_buffer, "0\n\0");
            full_mode = 0;
            break;
        case FULL_MODE:
            strcpy(settings_buffer, "1\n\0");
            full_mode = 1;
            break;
        default :
            printk(KERN_WARNING MODULE_TAG_D" settings: failed to update mode");
    }
    return (ssize_t) count;
}