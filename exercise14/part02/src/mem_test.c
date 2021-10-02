#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/slab.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Al_Sah");
MODULE_DESCRIPTION("Simple memory testing module");
MODULE_VERSION("0.1");

#define MODULE_TAG "memory_tester"

void run_kmalloc(void);
void run_kcalloc(void);
void run_kzalloc(void);
void run_vmalloc(void);
void run_vzalloc(void);

void run_get_free_pages(void);
void run_alloc_pages(void);


static int __init memory_tester_init(void)
{
    printk(KERN_NOTICE MODULE_TAG " Loaded\n");

    run_kmalloc();
    run_kcalloc();
    run_kzalloc();

    //KMALLOC_MAX_SIZE

    run_vmalloc();
    run_vzalloc();

    run_get_free_pages();
    run_alloc_pages();

    return -1;
}

/*static void __exit memory_tester_exit(void) {
    printk(KERN_NOTICE MODULE_TAG " Exited\n");
}
module_exit(memory_tester_exit)*/
module_init(memory_tester_init)



void run_kmalloc(void){
    s64 allocation_size = 2;
    while(1){
        char * allocated_memory = (char *)kmalloc(allocation_size, GFP_KERNEL);
        if(allocated_memory == NULL){
            printk(" [kmalloc] Failed to allocate %lld bytes\n", allocation_size);
            break;
        }
        printk(" [kmalloc] Allocated % 12lld bytes\n", allocation_size);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        kfree(allocated_memory);
    }
}

void run_kcalloc(void){
    s64 allocation_size = 2;

    while(1){
        char * allocated_memory = (char *)kcalloc( sizeof(char), allocation_size, GFP_KERNEL);
        if(allocated_memory == NULL){
            printk(" [kcalloc] Failed to allocate %lld bytes\n", allocation_size);
            break;
        }
        printk(" [kcalloc] Allocated % 12lld bytes\n", allocation_size);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        kfree(allocated_memory);
    }
}

void run_kzalloc(void){
    s64 allocation_size = 2;

    while(1){
        char * allocated_memory = (char *)kzalloc( allocation_size, GFP_KERNEL);
        if(allocated_memory == NULL){
            printk(" [kzalloc] Failed to allocate %lld bytes\n", allocation_size);
            break;
        }
        printk(" [kzalloc] Allocated % 12lld bytes\n", allocation_size);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        kfree(allocated_memory);
    }
}


void run_vmalloc(void){
    s64 allocation_size = 2;

    while(1){
        char * allocated_memory = (char *)vmalloc(allocation_size);

        if(allocated_memory == NULL){
            printk(" [vmalloc] Failed to allocate %lld bytes\n", allocation_size);
            break;
        }
        printk(" [vmalloc] Allocated % 12lld bytes\n", allocation_size);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        vfree(allocated_memory);
    }
}

void run_vzalloc(void){
    s64 allocation_size = 2;

    while(1){
        char * allocated_memory = (char *)vzalloc(allocation_size);

        if(allocated_memory == NULL){
            printk(" [vzalloc] Failed to allocate %lld bytes\n", allocation_size);
            break;
        }
        printk(" [vzalloc] Allocated % 12lld bytes\n", allocation_size);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        vfree(allocated_memory);
    }
}

void run_get_free_pages(void){
    u32 allocation_size = 1;
    ulong page;

    while(1){
        page = __get_free_pages(GFP_KERNEL, allocation_size);

        if(!page){
            printk(" [__get_free_pages] Failed to allocate %u pages\n", allocation_size);
            break;
        }
        printk(" [__get_free_pages] Allocated %u pages\n", allocation_size);

        free_pages(page, allocation_size);
        allocation_size +=1;
    }
}

void run_alloc_pages(void){
    u32 allocation_size = 1;
    struct page * page;

    while(1){
        page = alloc_pages(GFP_KERNEL, allocation_size);

        if(!page){
            printk(" [alloc_pages] Failed to allocate %u pages\n", allocation_size);
            break;
        }
        printk(" [alloc_pages] Allocated %u pages\n", allocation_size);

        __free_pages(page, allocation_size);
        allocation_size +=1;
    }
}
