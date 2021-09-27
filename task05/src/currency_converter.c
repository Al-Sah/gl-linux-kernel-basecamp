#include "../currency_converter.h"


// Euro will be a basic currency
static struct class *currency_convertor = NULL;
static struct proc_dir_entry *proc_dir = NULL;

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


CLASS_ATTR( del_currency, ( S_IWUSR | S_IRUGO ), &sys_currency_manager_show, &sys_currency_manager_store );
CLASS_ATTR( add_currency, ( S_IWUSR | S_IRUGO ), &sys_currency_manager_show, &sys_currency_manager_store );


int init_sys_interface(void){
    int error;
    currency_convertor = class_create(THIS_MODULE, SYS_CLASS);
    if (IS_ERR(currency_convertor)) {
        printk(KERN_WARNING ": Failed to create %s class", SYS_CLASS);
        return 1;
    }

    error = class_create_file(currency_convertor, &class_attr_del_currency);
    if(error){
        printk(KERN_WARNING ": Failed to create sys_fs 'del_currency' file");
        return error;
    }
    error = class_create_file(currency_convertor, &class_attr_add_currency);
    if(error){
        printk(KERN_WARNING ": Failed to create sys_fs 'add_currency' file");
        return error;
    }
    return error;
}

void fast_conversion(void){
    struct currency currency[] = {
            create("EUR", 1, 1, 1, 1),
            create("UAH", 31379, 1000000, 319108, 10000),
            create("RUB", 1154, 100000, 86576, 1000)
    };
    struct currency *in = NULL, *out = NULL;
    int i;

    printk(KERN_INFO "task05: Hello user !! It is currency convertor\n");
    printk(KERN_INFO "task05: Input amount: %ld\n", (long)amount);
    printk(KERN_INFO "task05: Converting from %s to %s", in_currency, out_currency);

    for ( i = 0; i < 3; ++i) {
        if (strcmp(in_currency, currency[i].id) == 0) {
            in = &currency[i];
        }
    }
    if(in == NULL){
        printk(KERN_WARNING ": Currency '%s' not found", in_currency);
        return;
    }
    for ( i = 0; i < 3; ++i) {
        if (strcmp(out_currency, currency[i].id) == 0) {
            out = &currency[i];
        }
    }
    if(out == NULL){
        printk(KERN_WARNING ": Currency '%s' not found", out_currency);
        return;
    }
    printk(KERN_INFO ": Converting %s to %s; Amount: %ld. Res: %s\n",
             in_currency, out_currency, amount, convert(amount, in, out));
}

int __init currency_convertor_init(void)
{
    int error;
    if(strlen(in_currency) != 0 && strlen(out_currency) != 0){
        fast_conversion();
        return -1;
    }

    error = init_proc_interface();
    if(error){
        return error;
    }
    error = init_sys_interface();
    if(error){
        return error;
    }
    return 0;
}






struct currency create(char* id,
                       int to_main_currency_multiplier, int to_eur_div,
                       int from_eur_mul, int from_eur_div ){

    struct currency currency;
    struct conversion_rule from_eur;
    struct conversion_rule to_eur;

    to_eur.divider = to_eur_div;
    to_eur.multiplier = to_main_currency_multiplier;
    from_eur.multiplier = from_eur_mul;
    from_eur.divider = from_eur_div;

    strcpy(currency.id, id);
    currency.from_main = from_eur;
    currency.to_main = to_eur;

    return currency;
}

void delete_currencies(void){
    struct currency_list *temp;
    struct list_head *cursor, *temp_storage;

    list_for_each_safe(cursor, temp_storage, &head_node){
        temp = list_entry(cursor, struct currency_list, list);
        list_del(cursor);
        if(proc_dir != NULL && temp->proc != NULL){
            remove_proc_entry(temp->data.id, proc_dir);
        }
        class_remove_file(currency_convertor, &temp->attribute);
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

    printk(KERN_INFO " DELETING %s\n", id);

    list_for_each_safe(cursor, temp_storage, &head_node){
        temp = list_entry(cursor, struct currency_list, list);
        if(strcmp(temp->data.id, id) == 0){
            list_del(cursor);
            if(proc_dir != NULL && temp->proc != NULL){
                remove_proc_entry(temp->data.id, proc_dir);
            }
            class_remove_file(currency_convertor, &temp->attribute);
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

int update_currency(struct currency currency){

    struct currency *old_currency = find_currency(currency.id);
    if(old_currency == NULL){
        return 1;
    }
    old_currency->to_main = currency.to_main;
    old_currency->from_main = currency.from_main;
    return 0;
}


inline int init_proc_interface(void)
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
    if (proc_dir != NULL){
        res = proc_create(name, 0, proc_dir, &proc_io_fops);
    }
    return res;
}

static void parse_conversion_factor(struct currency *currency, int value, ushort position){
    switch (position){
        case 1:
            currency->to_main.multiplier = value;
            break;
        case 2:
            currency->to_main.divider = value;
            break;
        case 3:
            currency->from_main.multiplier = value;
            break;
        case 4:
            currency->from_main.divider = value;
            break;
        default:
            printk( KERN_ERR " parse_conversion_factor fails");
            break;
    }
}


static struct currency get_currency_from_str(const char* buffer){
    char *token = NULL, *substr = NULL;
    struct currency new_currency = {};
    long from_str;
    size_t i;

    token = buffer; // FIXME
    for(i = 0; i < 5; ++i){
        substr = strsep(&token, " ");
        if(substr == NULL){
            break;
        }
        if(i == 0){
            strcpy(new_currency.id, substr);
            continue;
        }
        if(kstrtol(substr, 10, &from_str) == 0){
            parse_conversion_factor(&new_currency, (int)from_str, i);
        }
    }
    return new_currency;
}


struct class_attribute get_class_attribute(const char *name){
    struct class_attribute class_attr_node = {
            .attr = {.name = name, .mode = VERIFY_OCTAL_PERMISSIONS(( S_IWUSR | S_IRUGO )) },
            .show	= sys_currency_show,
            .store	= sys_currency_store,
    };
    return class_attr_node;
}


void add_new_currency(struct currency currency){
    struct currency_list *new_node = kmalloc(sizeof(struct currency_list), GFP_KERNEL);
    new_node->data = currency;
    new_node->proc = create_proc_entry(currency.id);
    new_node->attribute = get_class_attribute(new_node->data.id);

    if (class_create_file(currency_convertor, &new_node->attribute) != 0) {
        printk(KERN_WARNING ": Failed to create sys_fs 'settings' file");
    }
    list_add_tail(&new_node->list, &head_node);
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
    while (res > DECIMAL_PLACES / shift) {
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

char* convert(ssize_t sum, const struct currency* in_curr, const struct currency* out_curr) {

    static char result_amount[16] = "";
    long res_int_part;
    long res_float_part;
    long int tmp;

    if(in_curr == NULL || out_curr == NULL){
        printk(KERN_INFO "Input contains null ptr: in %p, out %p", in_curr, out_curr);
        return "";
    }
    if(strcmp(in_curr->id, out_curr->id) == 0){
        snprintf(result_amount, sizeof(result_amount), "%ld\n", (long)sum);
        return result_amount;
    }

    tmp = (long long int)sum * (long long int)in_curr->to_main.multiplier * (long long int)out_curr->from_main.multiplier;

    res_int_part = tmp / (in_curr->to_main.divider * out_curr->from_main.divider);
    res_float_part = tmp % (in_curr->to_main.divider * out_curr->from_main.divider);

    if (sum * in_curr->to_main.multiplier * out_curr->from_main.multiplier <
        in_curr->to_main.divider * out_curr->from_main.divider) {

        short zeros = get_zeros(sum * in_curr->to_main.multiplier * out_curr->from_main.multiplier,
                                in_curr->to_main.divider * out_curr->from_main.divider);

        snprintf(result_amount, sizeof(result_amount), "%ld.%s%d\n", res_int_part, get_zeros_str(zeros), get_float_part(res_float_part,(uint32_t)power(10, zeros)));
    } else {
        snprintf(result_amount, sizeof(result_amount), "%ld.%d\n", res_int_part, get_float_part(res_float_part,1));
    }
    return result_amount;
}

ssize_t proc_read(struct file *file_p, char __user *buffer, size_t length, loff_t *offset)
{
    static char conversion_result[1000];
    if( *offset != 0 ) {
        return 0;
    }
    strcpy(out_currency, file_p->f_path.dentry->d_iname);
    snprintf(conversion_result, sizeof(conversion_result),
             "Converting %s to %s; Amount: %ld. Res: %s\n",
             in_currency, out_currency, amount,
             convert(amount, find_currency(in_currency), find_currency(out_currency)));
    length = strlen(conversion_result);

    if( copy_to_user( buffer, conversion_result, length)) {
        printk(KERN_WARNING ": Failed to copy some chars");
        return -EINVAL;
    }
    printk(KERN_WARNING ": PROC READ %lu", (ulong)length);
    *offset = (loff_t)length;
    return (ssize_t)length;
}

ssize_t proc_write(struct file *file_p, const char __user *buffer, size_t length, __attribute__((unused)) loff_t *offset)
{
    size_t failed;
    long res = 0;

    char tmp_buffer[CURRENCY_SIZE];
    if(length > CURRENCY_SIZE){
        length = CURRENCY_SIZE;
    }
    failed = copy_from_user(tmp_buffer, buffer, length);
    tmp_buffer[length] = '\0';
    printk(KERN_WARNING ": tmp_buffer '%s' \n", tmp_buffer);

    if(kstrtol(tmp_buffer, 10, &res) == 0){
        printk(KERN_WARNING ": NEW amount %lu \n", (ulong)amount);
        amount = res;
    }else{
        printk(KERN_WARNING ": FAILED to set new amount %lu \n", (ulong)res);
    }
    strcpy(in_currency, file_p->f_path.dentry->d_iname);
    if (failed != 0){
        printk(KERN_WARNING ": Failed to copy %lu from %lu \n", (ulong)failed, (ulong)length);
    }
    return (ssize_t)length;
}



void __exit currency_convertor_exit(void)
{
    delete_currencies();
    remove_proc_entry(PROC_DIRECTORY, NULL);
    class_remove_file(currency_convertor, &class_attr_add_currency );
    class_remove_file(currency_convertor, &class_attr_del_currency );
    class_destroy(currency_convertor);
    printk(KERN_INFO "task05: Bye...\n");
}

module_init(currency_convertor_init)
module_exit(currency_convertor_exit)


ssize_t sys_currency_show(__attribute__((unused)) struct class *class, struct class_attribute *attr, char *buf ) {

    struct currency *res = find_currency(attr->attr.name);
    if(res != NULL){
        sprintf(buf, "%s %d %d %d %d", res->id,
                res->to_main.multiplier, res->to_main.divider,
                res->from_main.multiplier, res->from_main.divider);
    } else {
        sprintf(buf, "Currency not found '%s'", attr->attr.name);
    }
    return (ssize_t)strlen(buf);
}

ssize_t sys_currency_store(__attribute__((unused)) struct class *class, struct class_attribute *attr, const char *buf, size_t count ) {

    char *token = NULL, *substr = NULL;
    struct currency currency = create("", 0,0,0,0);
    long from_str;
    size_t i;
    strcpy(currency.id , attr->attr.name);
    token = buf; // FIXME
    for(i = 1; i < 5; ++i){
        substr = strsep(&token, " ");
        if(substr == NULL){
            printk( KERN_INFO " UPDATE _ substr == NULL");
            break;
        }
        if(kstrtol(substr, 10, &from_str) == 0){
            parse_conversion_factor(&currency, (int)from_str, i);
        }
    }
    update_currency(currency);
    return (ssize_t)count;
}


ssize_t sys_currency_manager_show(__attribute__((unused)) struct class *class, __attribute__((unused)) struct class_attribute *attr, char *buf ) {
    printk(KERN_ERR " currency_manager_show: file %s", attr->attr.name);
    if(strcmp(attr->attr.name, DEL_CURRENCY_FILE) == 0){
        strcpy(buf, " You have to write currency here, like RUB, UAH, EUR ...\n");
    } else if(strcmp(attr->attr.name, ADD_CURRENCY_FILE) == 0){
        strcpy(buf, " You have to write currency here in next format: 'ID to_x_mul to_x_del from_x_mul from_x_del\n");
    } else{
        printk(KERN_ERR " currency_manager_show: undefined file ??");
    }
    return (ssize_t)strlen(buf);
}

ssize_t sys_currency_manager_store(__attribute__((unused)) struct class *class, struct class_attribute *attr, const char *buf, size_t count ) {

    printk(KERN_INFO " DELETING %s\n", attr->attr.name);
    if(strcmp(attr->attr.name, DEL_CURRENCY_FILE) == 0){
        char currency_id[10];
        count = count > 10 ? 10 : count;
        strncpy(currency_id, buf, count);
        if(currency_id[count-1] == '\n'){
            currency_id[count-1] = '\0';
        }
        printk(KERN_INFO " DELETING %s\n", currency_id);
        delete_currency(currency_id);
    }else if(strcmp(attr->attr.name, ADD_CURRENCY_FILE) == 0){
        struct currency new = get_currency_from_str(buf);
        if(find_currency(new.id) == NULL){
            add_new_currency(new);
        } else {
            printk(KERN_ERR " currency_manager_show: currency exists");
        }

    } else{
        printk(KERN_ERR " currency_manager_show: undefined file ??");
    }

    return (ssize_t)count;
}