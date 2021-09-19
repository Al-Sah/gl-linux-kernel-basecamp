#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple currency convertor");
MODULE_VERSION("0.1");

// Euro will be a basic currency
#define MAX_CURRENCY_STR_SIZE 6
#define FLOAT_PART_SIZE 1000


struct list_head head_node = LIST_HEAD_INIT(head_node);


struct conversion_rule{
    int multiplier;
    int divider;
};

struct currency {
    char id[MAX_CURRENCY_STR_SIZE];
    struct conversion_rule to_eur;
    struct conversion_rule from_eur;
};

struct currency_list{
    struct list_head list;
    struct currency data;
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
        kfree(temp);
    }
    if(!list_empty(&head_node)){
        printk(KERN_WARNING "List is not empty after freeing!");
    } else {
        printk(KERN_WARNING "List cleaned");
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

void add_list_node(struct currency currency){
    struct currency_list *temp_node = NULL;

    temp_node = kmalloc(sizeof(struct currency_list), GFP_KERNEL);
    temp_node->data = currency;
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

static int money = 100;
static char *in_currency = "UAH";
static char *out_currency = "EUR";

module_param(money, int, S_IRUSR);
MODULE_PARM_DESC(money, "A number to convert");

module_param(in_currency, charp, 0000);
MODULE_PARM_DESC(in_currency, "Currency \"from\" ");

module_param(out_currency, charp, 0000);
MODULE_PARM_DESC(out_currency, "Currency \"to\"");


static short get_zeros(unsigned int a, unsigned int b ){
    short zeros = 0;
    a *= 10;
    while (a < b){
        a *= 10;
        zeros++;
    }
    return zeros;
}

static void convert(int sum, const struct currency* in_curr, const struct currency* out_curr) {

    char result_amount[16] = "";
    long res_int_part;
    long res_float_part;
    long int tmp;

    if(in_curr == NULL || out_curr == NULL){
        printk(KERN_INFO "Input contains null ptr: in %p, out %p", in_curr, out_curr);
        return;
    }

    tmp = (long long int)sum * (long long int)in_curr->to_eur.multiplier * (long long int)out_curr->from_eur.multiplier;

    res_int_part = tmp / (in_curr->to_eur.divider * out_curr->from_eur.divider);
    res_float_part = tmp % (in_curr->to_eur.divider * out_curr->from_eur.divider);

    if (sum * in_curr->to_eur.multiplier * out_curr->from_eur.multiplier <
        in_curr->to_eur.divider * out_curr->from_eur.divider) {

        short zeros = get_zeros(sum * in_curr->to_eur.multiplier * out_curr->from_eur.multiplier,
                                in_curr->to_eur.divider * out_curr->from_eur.divider);

        snprintf(result_amount, sizeof(result_amount), "%ld.%s%d", res_int_part, get_zeros_str(zeros), get_float_part(res_float_part,(uint32_t)power(10, zeros)));
    } else {
        snprintf(result_amount, sizeof(result_amount), "%ld.%d", res_int_part, get_float_part(res_float_part,1));
    }
    printk(KERN_INFO "Result: %s\n", result_amount);
}

static int __init task05_init(void)
{

    add_list_node(create("EUR", 1, 1, 1, 1));
    add_list_node(create("UAH", 31379, 1000000, 319108, 10000));
    add_list_node(create("RUB", 1154, 100000, 86576, 1000));


    printk(KERN_INFO "task05: Testing ....");
    convert(1, find_currency("UAH"), find_currency("EUR"));
    convert(1, find_currency("RUB"), find_currency("EUR"));
    convert(100, find_currency("UAH"), find_currency("RUB"));
    convert(100, find_currency("UAH"), find_currency("ZZZ"));

    printk(KERN_INFO "task05: Hello user !! It is currency convertor\n");
    printk(KERN_INFO "task05: Input amount of money: %d\n", money);
    printk(KERN_INFO "task05: Converting from %s to %s", in_currency, out_currency);

    convert(money, find_currency(in_currency), find_currency(out_currency));
    return 0;
}


static void __exit task05_exit(void)
{
    clean_list();
    printk(KERN_INFO "task05: Bye...\n");
}

module_init(task05_init)
module_exit(task05_exit)