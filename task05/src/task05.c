
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple currency convertor");
MODULE_VERSION("0.1");


static int money = 100;
static int result = 0, result_float=0;
static char *in_currency = "UAH";
static char *out_currency = "EUR";
static int success=0;

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
    zeros--;
    return zeros;
}

static void convert(int sum, char * in_currency, char * out_currency)
{
    char result_amount[16] = "";

    if((strcmp(in_currency, "UAH") == 0) && (strcmp(out_currency, "EUR") == 0)){

        int tmp = sum * 3102;
        result = tmp / 100000;
        result_float = tmp % 100000;

        short zeros = get_zeros(3102, 100000);

        if(zeros == 0){
            int res = result_float;
            while (res > 100){
                res /=10;
            }
            snprintf(result_amount, sizeof(result_amount), "%d.%d", result, res);
        } else if (zeros == 1){
            int res = result_float;
            while (res > 10){
                res /=10;
            }
            snprintf(result_amount, sizeof(result_amount), "%d.0%d", result, res);
        } else {
            snprintf(result_amount, sizeof(result_amount), "%d.00", result);
        }
        success=1;


    } else if((strcmp(in_currency, "EUR") == 0) && (strcmp(out_currency, "UAH") == 0)){
        int tmp = 32232;
        int res = 0;

        result = tmp / 1000;
        result_float = tmp % 1000;
        res = result_float;

        while (res > 100){
            res /= 10;
        }
        snprintf(result_amount, sizeof(result_amount), "%d.%d", result, res);
        success=1;
    }



    if(success == 1){
        printk(KERN_INFO "task05: Conversion result: %s", result_amount);
    }else{
        printk(KERN_INFO "task05: Failed to recognize currencies");
    }
}

static int __init task05_init(void)
{
    printk(KERN_INFO "task05: Hello user !! It is currency convertor\n");
    printk(KERN_INFO "task05: Input amount of money: %d\n", money);
    printk(KERN_INFO "task05: Converting from %s to %s", in_currency, out_currency);

    convert(money, in_currency, out_currency);

    return 0;
}


static void __exit task05_exit(void)
{
    printk(KERN_INFO "task05: Bye...\n");
}

module_init(task05_init);
module_exit(task05_exit);