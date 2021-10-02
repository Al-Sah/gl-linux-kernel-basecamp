#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/timekeeping.h>

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
    run_vmalloc();
    run_vzalloc();

    run_get_free_pages();
    run_alloc_pages();

    return -1;
}
module_init(memory_tester_init)



void run_kmalloc(void){
    s64 allocation_size = 2;
    char * allocated_memory = NULL;
    u64 alloc_ns, free_ns;
    while(1){
        alloc_ns = ktime_get_real_ns();
        allocated_memory = (char *)kmalloc(allocation_size, GFP_KERNEL);
        alloc_ns = ktime_get_real_ns() - alloc_ns;

        if(allocated_memory == NULL){
            printk(" [kmalloc]: Failed to allocate %lld bytes; alloc time: %llu\n", allocation_size, alloc_ns);
            break;
        }
        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_ns = ktime_get_real_ns();
        kfree(allocated_memory);
        free_ns = ktime_get_real_ns() - free_ns;

        printk(" [kmalloc]: Allocated % 12lld bytes; alloc time: % 10lld free time: % 10lld\n", allocation_size, alloc_ns, free_ns);
    }
}

void run_kcalloc(void){
    s64 allocation_size = 2;
    char * allocated_memory = NULL;
    u64 alloc_ns, free_ns;

    while(1){
        alloc_ns = ktime_get_real_ns();
        allocated_memory = (char *)kcalloc( sizeof(char), allocation_size, GFP_KERNEL);
        alloc_ns = ktime_get_real_ns() - alloc_ns;

        if(allocated_memory == NULL){
            printk(" [kcalloc]: Failed to allocate %lld bytes; alloc time: %llu\n", allocation_size, alloc_ns);
            break;
        }

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_ns = ktime_get_real_ns();
        kfree(allocated_memory);
        free_ns = ktime_get_real_ns() - free_ns;

        printk(" [kcalloc]: Allocated % 12lld bytes; alloc time: % 10lld free time: % 10lld\n", allocation_size, alloc_ns, free_ns);
    }
}

void run_kzalloc(void){
    s64 allocation_size = 2;
    char * allocated_memory = NULL;
    u64 alloc_ns, free_ns;

    while(1){
        alloc_ns = ktime_get_real_ns();
        allocated_memory = (char *)kzalloc( allocation_size, GFP_KERNEL);
        alloc_ns = ktime_get_real_ns() - alloc_ns;

        if(allocated_memory == NULL){
            printk(" [kzalloc]: Failed to allocate %lld bytes; alloc time: %llu\n", allocation_size, alloc_ns);
            break;
        }

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_ns = ktime_get_real_ns();
        kfree(allocated_memory);
        free_ns = ktime_get_real_ns() - free_ns;

        printk(" [kzalloc]: Allocated % 12lld bytes; alloc time: % 10lld free time: % 10lld", allocation_size, alloc_ns, free_ns);
    }
}


void run_vmalloc(void){
    s64 allocation_size = 2;
    char * allocated_memory = NULL;
    u64 alloc_ns, free_ns;

    while(1){
        alloc_ns = ktime_get_real_ns();
        allocated_memory = (char *)vmalloc(allocation_size);
        alloc_ns = ktime_get_real_ns() - alloc_ns;


        if(allocated_memory == NULL){
            printk(" [vmalloc]: Failed to allocate %lld bytes; alloc time: %llu\n", allocation_size, alloc_ns);
            break;
        }

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_ns = ktime_get_real_ns();
        vfree(allocated_memory);
        free_ns = ktime_get_real_ns() - free_ns;

        printk(" [vmalloc]: Allocated % 12lld bytes; alloc time: % 10lld free time: % 10lld\n", allocation_size, alloc_ns, free_ns);
    }
}

void run_vzalloc(void){
    s64 allocation_size = 2;
    char * allocated_memory = NULL;
    u64 alloc_ns, free_ns;

    while(1){
        alloc_ns = ktime_get_real_ns();
        allocated_memory = (char *)vzalloc(allocation_size);
        alloc_ns = ktime_get_real_ns() - alloc_ns;

        if(allocated_memory == NULL){
            printk(" [vzalloc]: Failed to allocate %lld bytes; alloc time: %llu\n", allocation_size, alloc_ns);
            break;
        }

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_ns = ktime_get_real_ns();
        vfree(allocated_memory);
        free_ns = ktime_get_real_ns() - free_ns;

        printk(" [vzalloc]: Allocated % 12lld bytes; alloc time: % 10lld free time: % 10lld\n", allocation_size, alloc_ns, free_ns);
    }
}

void run_get_free_pages(void){
    u32 allocation_size = 1;
    ulong page;
    u64 alloc_ns, free_ns;

    while(1){
        alloc_ns = ktime_get_real_ns();
        page = __get_free_pages(GFP_KERNEL, allocation_size);
        alloc_ns = ktime_get_real_ns() - alloc_ns;

        if(!page){
            printk(" [__get_free_pages]: Failed to allocate %u pages\n", allocation_size);
            break;
        }

        free_ns = ktime_get_real_ns();
        free_pages(page, allocation_size);
        free_ns = ktime_get_real_ns() - free_ns;
        allocation_size +=1;
        printk(" [__get_free_pages]: Allocated % 3d pages; alloc time: % 10lld free time: % 10lld\n", allocation_size, alloc_ns, free_ns);
    }
}

void run_alloc_pages(void){
    u32 allocation_size = 1;
    struct page * page;
    u64 alloc_ns, free_ns;

    while(1){
        alloc_ns = ktime_get_real_ns();
        page = alloc_pages(GFP_KERNEL, allocation_size);
        alloc_ns = ktime_get_real_ns() - alloc_ns;

        if(!page){
            printk(" [alloc_pages]: Failed to allocate %u pages\n", allocation_size);
            break;
        }

        free_ns = ktime_get_real_ns();
        __free_pages(page, allocation_size);
        free_ns = ktime_get_real_ns() - free_ns;
        allocation_size +=1;
        printk(" [alloc_pages]: Allocated % 3d pages; alloc time: % 10lld free time: % 10lld\n", allocation_size, alloc_ns, free_ns);
    }
}
