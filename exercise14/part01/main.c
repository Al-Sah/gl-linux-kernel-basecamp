#include <stdio.h>
#include <malloc.h>
#include <alloca.h>

#include <time.h>

typedef long s64;

void run_malloc(void);
void run_calloc(void);
void run_alloca(void);

int main() {

    run_malloc();
    run_calloc();
    run_alloca();
    return 0;
}

void run_malloc(void){
    clock_t alloc_start, alloc_end, free_start, free_end;

    s64 allocation_size = 2;
    while(1){
        alloc_start = clock();
        char * allocated_memory = (char *)malloc(allocation_size);
        alloc_end = clock();

        if(allocated_memory == NULL){
            printf(" [malloc] Failed to allocate %ld bytes; %ld GB\n", allocation_size, allocation_size/1073741824);
            break;
        }
        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_start = clock();
        free(allocated_memory);
        free_end = clock();

        printf(" [malloc] Allocated % 12ld bytes; % 8ld MB; alloc time: %Lf\t free time: %Lf\n",
               allocation_size, allocation_size/1024,
               ((long double) (alloc_end - alloc_start)) / CLOCKS_PER_SEC,
               ((long double) (free_end - free_start)) / CLOCKS_PER_SEC);
    }
}

void run_calloc(void){
    clock_t alloc_start, alloc_end, free_start, free_end;
    s64 allocation_size = 2;

    while(1){
        alloc_start = clock();
        char * allocated_memory = (char *)calloc( sizeof(char), allocation_size);
        alloc_end = clock();

        if(allocated_memory == NULL){
            printf(" [calloc] Failed to allocate %ld bytes; %ld GB\n", allocation_size, allocation_size/1073741824);
            break;
        }

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free_start = clock();
        free(allocated_memory);
        free_end = clock();

        printf(" [calloc] Allocated % 12ld bytes; % 8ld MB; alloc time: %Lf\t free time: %Lf\n",
               allocation_size, allocation_size/1024,
               ((long double) (alloc_end - alloc_start)) / CLOCKS_PER_SEC,
               ((long double) (free_end - free_start)) / CLOCKS_PER_SEC);
    }
}

void run_alloca(void){
    s64 allocation_size = 2;
    clock_t alloc_start, alloc_end;

    while(1){
        alloc_start = clock();
        char * allocated_memory = (char *)alloca(allocation_size);
        alloc_end = clock();

        if(allocated_memory == NULL){
            break;
        }

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        printf(" [alloca] Allocated % 12ld bytes; % 4ld MB; alloc time: %Lf\n",
               allocation_size, allocation_size/1024,
               ((long double) (alloc_end - alloc_start)) / CLOCKS_PER_SEC);
    }
}