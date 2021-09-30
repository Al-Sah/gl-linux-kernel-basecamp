//
// Created by al_sah on 25.09.21.
//

#ifndef CURRENCY_CONVERTER_H
#define CURRENCY_CONVERTER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/fs.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple currency converter");
MODULE_VERSION("0.1");


int  __init currency_convertor_init(void);
void __exit currency_convertor_exit(void);


struct conversion_rule{
    int multiplier;
    int divider;
};

struct currency {
    char id[DNAME_INLINE_LEN];
    struct conversion_rule to_main;
    struct conversion_rule from_main;
};

struct currency_list{
    struct list_head list;
    struct currency data;
    struct proc_dir_entry *proc;
    struct class_attribute attribute;
};



/**
 * <b> Fast conversion mode </b><p>
 *
 * In fast conversion mode module will convert currencies and exit. <p>
 *
 * To enable fast mode module have to be loaded with 3 arguments: <p>
 * 1) 'amount' as an integer <p>
 * 2) 'in' - input currency <p>
 * 3) 'out' - Output currency <p>
 *
 * Result will be printed to dmesg
 */


long amount = 100;
char in_currency[DNAME_INLINE_LEN] = "";
char out_currency[DNAME_INLINE_LEN] = "";

module_param(amount, long, S_IRUSR);
MODULE_PARM_DESC(amount, "An amount to convert");

module_param_string(in, in_currency, DNAME_INLINE_LEN, 0);
MODULE_PARM_DESC(in_currency, "Input currency");

module_param_string(out, out_currency, DNAME_INLINE_LEN, 0);
MODULE_PARM_DESC(out_currency, "Output currency");



#define DECIMAL_PLACES  100
/**
 * @returns Static buffer with converted value
 * @note Number of decimal places (numbers after point) can be set via DECIMAL_PLACES macro
 */
char* convert(ssize_t amount, const struct currency* in, const struct currency* out);

struct currency create(char* currency_id,
        int to_main_currency_multiplier, int to_main_currency_delimiter,
        int from_main_currency_multiplier, int from_main_currency_delimiter);




/**
 * proc_fs interface
 */
#define PROC_DIRECTORY  "currency_converter"
#define CURRENCY_SIZE 10 ///< Number of digits

inline int init_proc_interface(void);

ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
ssize_t proc_write(struct file *file_p, const char __user *buffer, size_t length, __attribute__((unused)) loff_t *offset);




/**
 * sys_fs interface
 */
#define DEL_CURRENCY_FILE   "del_currency"
#define ADD_CURRENCY_FILE   "add_currency"
#define SYS_CLASS           "currency_converter"

#define CLASS_ATTR(_name, _mode, _show, _store) struct class_attribute class_attr_##_name = __ATTR(_name, _mode, _show, _store)

ssize_t sys_currency_show(__attribute__((unused)) struct class *class,  __attribute__((unused)) struct class_attribute *attr, char *buf );
ssize_t sys_currency_store(__attribute__((unused)) struct class *class,  __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count );

/**
 * @currency_manager sys_fs interface which handles 2 files:
 *
 * 1) del_currency <p>
 * 2) add_currency <p>
 */
ssize_t sys_currency_manager_show(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, char *buf );
ssize_t sys_currency_manager_store(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, const char *buf, size_t count );





/**
 *  <b> list functions </b><p>
 *
 *  delete* functions delete currency from list and call kfree
 */
void delete_currencies(void);
void delete_currency(const char *currency_id);
int  update_currency(struct currency currency);
void add_new_currency(struct currency currency);
struct currency * find_currency(const char *currency_id);



#endif //CURRENCY_CONVERTER_H