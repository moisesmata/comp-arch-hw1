#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPEAT 100000

void clflush(volatile void *p) {
    asm volatile("clflush (%0)" :: "r" (p));
}

uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return a | ((uint64_t)d << 32);
}

void memtest(FILE *f, int power) {
    uint64_t start, end, clock;
    size_t buffer_size = 1UL << power;  // 2^power bytes

    // Allocate a buffer of size 2^n bytes
    char *buffer = (char *)malloc(buffer_size);

    // Allocate another buffer of the same size to copy the data into
    char *buffer_copy = (char *)malloc(buffer_size);
    
    // Check to make sure that the buffers allocated correctly
    if (!buffer || !buffer_copy) {
        fprintf(stderr, "Failed to allocate %zu bytes\n", buffer_size);
        return;
    }
    
    // Initialize buffer with data
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = '1';
    }
    
    // Set clock to initially
    clock = 0;
    
    // Get multiple trials worth of data
    for (long rep = 0; rep < REPEAT; rep++) {

        // Flush the cache line before each measurement
        // Make sure to do this for the all of the data, which is 2^N bytes!
        // Do it 64 bytes at a time
        for (size_t i = 0; i < buffer_size; i += 64) {
            clflush(buffer + i);
            clflush(buffer_copy + i);
        }
        
        // Start the timer
        asm volatile("mfence");
        start = rdtsc();

        // Copy the data over
        memcpy(buffer_copy, buffer, buffer_size);

        // End the timer
        asm volatile("mfence");
        end = rdtsc();
        
        // Calculate the number of ticks
        uint64_t ticks = end - start;
        clock += ticks;
        
        // Export to a csv
        fprintf(f, "%lu", ticks);

        // Add a comma
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