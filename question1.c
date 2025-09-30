#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPEAT 1000000

void clflush(volatile void *p) {
    asm volatile("clflush (%0)" :: "r" (p));
}

uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("mfence");
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    asm volatile("mfence");
    return a | ((uint64_t)d << 32);
}

void memtest(FILE *f, int power) {
    uint64_t start, end, clock;
    size_t buffer_size = 1UL << power;  // 2^power bytes
    
    char *buffer = (char *)malloc(buffer_size);
    char *buffer_copy = (char *)malloc(buffer_size);
    
    if (!buffer || !buffer_copy) {
        fprintf(stderr, "Failed to allocate %zu bytes\n", buffer_size);
        return;
    }
    
    // Initialize buffer with data
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = '1';
    }
    
    clock = 0;
    
    for (long rep = 0; rep < REPEAT; rep++) {
        //flush the cache before each mesurement
        for (size_t i = 0; i < buffer_size; i += 64) {
            clflush(buffer + i);
            clflush(buffer_copy + i);
        }
        
        asm volatile("mfence");
        
        start = rdtsc();
        memcpy(buffer_copy, buffer, buffer_size);
        end = rdtsc();
        
        uint64_t ticks = end - start;
        clock += ticks;
        
        fprintf(f, "%lu", ticks);
        if(rep < (REPEAT - 1)){
            fputc(',', f);
        } 
    }
    fputc('\n', f);
    
    printf("Power %d (2^%d = %zu bytes) took %lu ticks total, avg: %lu ticks\n", 
           power, power, buffer_size, clock, clock / REPEAT);
    
    free(buffer);
    free(buffer_copy);
}

int main(int argc, char **argv) {
    printf("Memory Copy Benchmark\n");
    printf("--------------------------------\n");
    
    FILE *f = fopen("results.csv", "w");
    if (!f) {
        fprintf(stderr, "Failed to open results.csv\n");
        return 1;
    }
    
    int powers[] = {6,7,8,9,10,11,12,13,14,15,16,20,21};
    int num_powers = sizeof(powers) / sizeof(int);
    
    for(int i = 0; i < num_powers; i++){
        printf("Testing power %d (%d/%d)\n", powers[i], i+1, num_powers);
        memtest(f, powers[i]);
    }
    
    fclose(f);
    return 0;
}