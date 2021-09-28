#include <stdio.h>
#include <malloc.h>
#include <alloca.h>

typedef long s64;

void run_malloc(void);
void run_calloc(void);
void run_alloca(void);

int main() {
    printf("Hello, World!\n");

    run_malloc();
    run_calloc();
    run_alloca();

    return 0;
}

void run_malloc(void){
    s64 allocation_size = 2;
    while(1){
        char * allocated_memory = (char *)malloc(allocation_size);

        if(allocated_memory == NULL){
            printf(" [malloc] Failed to allocate %ld bytes; %ld GB\n", allocation_size, allocation_size/1073741824);
            break;
        }
        printf(" [malloc] Allocated % 12ld bytes; %ld MB\n", allocation_size, allocation_size/1024);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free(allocated_memory);
    }
}

void run_calloc(void){
    s64 allocation_size = 2;
    while(1){
        char * allocated_memory = (char *)calloc( sizeof(char), allocation_size);

        if(allocated_memory == NULL){
            printf(" [calloc] Failed to allocate %ld bytes; %ld GB\n", allocation_size, allocation_size/1073741824);
            break;
        }
        printf(" [calloc] Allocated % 12ld bytes; % 6ld MB\n", allocation_size, allocation_size/1024);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;

        free(allocated_memory);
    }
}

void run_alloca(void){
    s64 allocation_size = 2;
    while(1){
        char * allocated_memory = (char *)alloca(allocation_size);

        if(allocated_memory == NULL){
            break;
        }
        printf(" [alloca] Allocated % 12ld bytes; % 6ld MB\n", allocation_size, allocation_size/1024);

        allocated_memory[allocation_size/2] = 111;
        allocation_size *=2;
    }
}