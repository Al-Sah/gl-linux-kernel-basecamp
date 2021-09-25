#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

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

#define MODULE_TAG      "time_manager"
#define MODULE_TAG_D    MODULE_TAG": "

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple time manager");
MODULE_VERSION("0.1");


int  __init time_manager_init(void);
void __exit time_manager_exit(void);



#define EACH_SECONDS 20
/** Generates random number each N seconds and prints it to dmesg <p>
 *  N - defined with EACH_SECONDS macro
**/
void timer_callback(__attribute__((unused)) struct timer_list *data);



/// ***************************************
/// <p><b> Time conversion </b><p>
/// ***************************************

#define NS_IN_SEC       1000000000
#define NS_IN_MS        1000000
#define NS_IN_US        1000

#define TIME_STR_LONG_SIZE  24 ///< 'hh:mm:ss ms:us:ns' format
#define TIME_STR_SIZE       10 ///< hh:mm:ss format
#ifdef CONFIG_X86_64
/**
 * @returns static buffer in 'hh:mm:ss ms:us:ns' format
 */
char* get_time_str_ns(u64 ns);
#endif
/**
 * @returns static buffer in 'hh:mm:ss' format
 */
char* get_time_str(size_t seconds_to_convert);




/// ***************************************
/// <p><b> proc_fs interface </b><p>
/// ***************************************

#define PROC_DIRECTORY          "time_manager"
#define PROC_TIMESTAMPS_FILE    "timestamps"
#define PROC_LAST_ACCESS_FILE   "last_access"
#define PROC_SET_TIME_FILE      "time_controller"

ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset);

ssize_t proc_time_controller_read(__attribute__((unused)) struct file *, char __user *, size_t , loff_t *);
ssize_t proc_time_controller_write(__attribute__((unused)) struct file *, const char __user *, size_t,
                                   __attribute__((unused)) loff_t *);

int  create_proc_entries(void);
void remove_proc_entries(void);





/// ***************************************
/** <b> sys_fs interface </b><p>
 *
 * To chose output format, user have to write in sys_fs file
 * <b>SHORT_MODE</b> or <b>FULL_MODE</b> macros value
**/
/// ***************************************


#define SHORT_MODE  '0'         ///< enable output in seconds
#define FULL_MODE   '1'         ///< enable output in hh:mm:ss form

#define CLASS_NAME  "time_manager"
#define SETTINGS_BUFFER_SIZE 4  ///< sys_fs settings buffer size

inline int create_sys_entries(void);

ssize_t sys_show(__attribute__((unused)) struct class *class,
        __attribute__((unused)) struct class_attribute *attr, char *buf );

ssize_t sys_store(__attribute__((unused)) struct class *class,
        __attribute__((unused)) struct class_attribute *attr,
        const char *buf, size_t count );

#define CLASS_ATTR(_name, _mode, _show, _store) \
    struct class_attribute class_attr_##_name = __ATTR(_name, _mode, _show, _store)

#endif