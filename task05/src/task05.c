#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/fs.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple currency convertor");
MODULE_VERSION("0.1");


// Euro will be a basic currency

static long money = 100;
static char in_currency[DNAME_INLINE_LEN] = "UAH";
static char out_currency[DNAME_INLINE_LEN] = "EUR";

module_param(money, long, S_IRUSR);
MODULE_PARM_DESC(money, "A number to convert");

module_param_string(in_currency, in_currency, DNAME_INLINE_LEN, 0);
MODULE_PARM_DESC(in_currency, "Currency \"from\" ");

module_param_string(out_currency, out_currency, DNAME_INLINE_LEN, 0);
MODULE_PARM_DESC(out_currency, "Currency \"to\"");

// Number of digits
#define MAX_CURRENCY_DIGITS 20
#define FLOAT_PART_SIZE 1000
#define PROC_DIRECTORY  "currency_convertor"


static struct proc_dir_entry *proc_dir;

static ssize_t proc_read(__attribute__((unused)) struct file *file_p, char __user *buffer, size_t length, loff_t *offset);
static ssize_t proc_write(__attribute__((unused)) struct file *file_p, const char __user *buffer, size_t length, __attribute__((unused)) loff_t *offset);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops proc_io_fops = {
    .proc_write = proc_write,
    .proc_read  = proc_read,
};
#else
static struct file_operations proc_io_fops = {
        .write = proc_write,
        .read  = proc_read
};
#endif


struct list_head head_node = LIST_HEAD_INIT(head_node);


struct conversion_rule{
    int multiplier;
    int divider;
};

struct currency {
    char id[DNAME_INLINE_LEN];
    struct conversion_rule to_eur;
    struct conversion_rule from_eur;
};

struct currency_list{
    struct list_head list;
    struct currency data;
    struct proc_dir_entry *proc;
};

struct currency create(char* id,
                       int to_eur_mul, int to_eur_div,
                       int from_eur_mul, int from_eur_div ){

    struct currency currency;
    struct conversion_rule from_eur;
    struct conversion_rule to_eur;

    to_eur.divider = to_eur_div;
    to_eur.multiplier = to_eur_mul;
    from_eur.multiplier = from_eur_mul;
    from_eur.divider = from_eur_div;

    strcpy(currency.id, id);
    currency.from_eur = from_eur;
    currency.to_eur = to_eur;

    return currency;
}

void clean_list(void){
    struct currency_list *temp;
    struct list_head *cursor, *temp_storage;

    list_for_each_safe(cursor, temp_storage, &head_node){
        temp = list_entry(cursor, struct currency_list, list);
        list_del(cursor);
        if(proc_dir != NULL && temp->proc != NULL){
            remove_proc_entry(temp->data.id, proc_dir);
        }
        kfree(temp);
    }
    if(!list_empty(&head_node)){
        printk(KERN_WARNING "List is not empty after freeing!");
    } else {
        printk(KERN_WARNING "List cleaned");
    }
}


void delete_currency(const char *id){
    struct currency_list *temp;
    struct list_head *cursor, *temp_storage;

    list_for_each_safe(cursor, temp_storage, &head_node){
        temp = list_entry(cursor, struct currency_list, list);
        if(strcmp(temp->data.id, id) == 0){
            list_del(cursor);
            if(proc_dir != NULL && temp->proc != NULL){
                remove_proc_entry(temp->data.id, proc_dir);
            }
            kfree(temp);
        }
    }
}


struct currency * find_currency(const char *id){
    struct currency_list *temp = NULL, *result = NULL;
    struct list_head *cursor, *temp_storage;

    list_for_each_safe(cursor, temp_storage, &head_node){
        temp = list_entry(cursor, struct currency_list, list);
        if(strcmp(temp->data.id, id) == 0){
            result = temp;
        }
    }
    if(result == NULL){
        return NULL;
    }
    return &result->data;
}


static inline int init_proc_interface(void)
{
    proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
    if (proc_dir == NULL){
        return -EFAULT;
    }
    return 0;
}


static inline struct proc_dir_entry * create_proc_entry(const char *name)
{
    static struct proc_dir_entry *res = NULL;
    if (proc_dir == NULL){
        return NULL;
    }
    res = proc_create(name, 0, proc_dir, &proc_io_fops);
    return res;
}


void add_list_node(struct currency currency){
    struct currency_list *temp_node = NULL;

    temp_node = kmalloc(sizeof(struct currency_list), GFP_KERNEL);
    temp_node->data = currency;
    temp_node->proc = create_proc_entry(currency.id);
    list_add_tail(&temp_node->list, &head_node);
}


int power(int x, short y)
{
    if (y == 0)
        return 1;
    else if (y%2 == 0)
        return power(x, (short)(y/2))*power(x, (short)(y/2));
    else
        return x*power(x, (short)(y/2))*power(x, (short)(y/2));
}


static const char* get_zeros_str(short zeros){
    static char str[16];
    int i;
    for(i = 0; i < zeros; i++){
        str[i] = '0';
    }
    str[++i] = '\0';
    return str;
}


static int get_float_part(long res_float_part, uint32_t shift){
    long res = res_float_part;
    while (res > FLOAT_PART_SIZE / shift) {
        res /= 10;
    }
    return (int)res;
}


static short get_zeros(unsigned int a, unsigned int b ){
    short zeros = 0;
    a *= 10;
    while (a < b){
        a *= 10;
        zeros++;
    }
    return zeros;
}

static char* convert(long sum, const struct currency* in_curr, const struct currency* out_curr) {

    static char result_amount[16] = "";
    long res_int_part;
    long res_float_part;
    long int tmp;

    if(in_curr == NULL || out_curr == NULL){
        printk(KERN_INFO "Input contains null ptr: in %p, out %p", in_curr, out_curr);
        return "";
    }
    if(strcmp(in_curr->id, out_curr->id) == 0){
        snprintf(result_amount, sizeof(result_amount), "%ld\n", money);
        return result_amount;
    }

    tmp = (long long int)sum * (long long int)in_curr->to_eur.multiplier * (long long int)out_curr->from_eur.multiplier;

    res_int_part = tmp / (in_curr->to_eur.divider * out_curr->from_eur.divider);
    res_float_part = tmp % (in_curr->to_eur.divider * out_curr->from_eur.divider);

    if (sum * in_curr->to_eur.multiplier * out_curr->from_eur.multiplier <
        in_curr->to_eur.divider * out_curr->from_eur.divider) {

        short zeros = get_zeros(sum * in_curr->to_eur.multiplier * out_curr->from_eur.multiplier,
                                in_curr->to_eur.divider * out_curr->from_eur.divider);

        snprintf(result_amount, sizeof(result_amount), "%ld.%s%d\n", res_int_part, get_zeros_str(zeros), get_float_part(res_float_part,(uint32_t)power(10, zeros)));
    } else {
        snprintf(result_amount, sizeof(result_amount), "%ld.%d\n", res_int_part, get_float_part(res_float_part,1));
    }
    return result_amount;
}

static ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
{
    static char conversion_result[1000];
    if( *offset != 0 ) {
        return 0;
    }
    strcpy(out_currency, file_p->f_path.dentry->d_iname);
    snprintf(conversion_result, sizeof(conversion_result),
             "Converting %s to %s; Amount: %ld. Res: %s\n",
             in_currency, out_currency, money,
             convert(money, find_currency(in_currency), find_currency(out_currency)));
    length = strlen(conversion_result);

    if( copy_to_user( buffer, conversion_result, length)) {
        printk(KERN_WARNING ": Failed to copy some chars");
        return -EINVAL;
    }
    printk(KERN_WARNING ": PROC READ %lu", (ulong)length);
    *offset = (loff_t)length;
    return (ssize_t)length;
}

static ssize_t proc_write(struct file *file_p, const char __user *buffer, size_t length, __attribute__((unused)) loff_t *offset)
{
    size_t failed;
    long res;

    char tmp_buffer[MAX_CURRENCY_DIGITS];
    if(length > MAX_CURRENCY_DIGITS){
        length = MAX_CURRENCY_DIGITS;
    }
    failed = copy_from_user(tmp_buffer, buffer, length);

    if(kstrtol(tmp_buffer, 10, &res) == 0){
        money = res;
    }
    strcpy(in_currency, file_p->f_path.dentry->d_iname);
    if (failed != 0){
        printk(KERN_WARNING ": Failed to copy %lu from %lu \n", (ulong)failed, (ulong)length);
    }
    return (ssize_t)length;
}

static int __init task05_init(void)
{
    int error = init_proc_interface();
    if(error){
       return error;
    }

    add_list_node(create("EUR", 1, 1, 1, 1));
    add_list_node(create("UAH", 31379, 1000000, 319108, 10000));
    add_list_node(create("RUB", 1154, 100000, 86576, 1000));


    printk(KERN_INFO "task05: Testing ....");
    convert(1, find_currency("UAH"), find_currency("EUR"));
    convert(1, find_currency("RUB"), find_currency("EUR"));
    convert(100, find_currency("UAH"), find_currency("RUB"));
    convert(100, find_currency("UAH"), find_currency("ZZZ"));

    printk(KERN_INFO "task05: Hello user !! It is currency convertor\n");
    printk(KERN_INFO "task05: Input amount of money: %ld\n", (long) money);
    printk(KERN_INFO "task05: Converting from %s to %s", in_currency, out_currency);

    convert(money, find_currency(in_currency), find_currency(out_currency));
    return 0;
}


static void __exit task05_exit(void)
{
    clean_list();
    remove_proc_entry(PROC_DIRECTORY, NULL);
    printk(KERN_INFO "task05: Bye...\n");
}

module_init(task05_init)
module_exit(task05_exit)