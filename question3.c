#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPEAT 1
#define ROW_BUFFER_SIZE (8 * 1024)  //depends on your system (DRAM Row buffer size)

static inline void clflush(volatile void *p) {
    asm volatile ("clflush (%0)" :: "r"(p));
}

static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile ("mfence");
    asm volatile ("rdtsc" : "=a" (a), "=d" (d));
    asm volatile ("mfence");
    return ((uint64_t)d << 32) | a;
}

char *buffer;
char *buffer_copy;

void memtest() {
    uint64_t start, end;
    uint64_t total_first = 0, total_second = 0;

    buffer = (char *)malloc(ROW_BUFFER_SIZE);
    buffer_copy = (char *)malloc(ROW_BUFFER_SIZE);

    // Initialize buffer to some data
    memset(buffer, 'A', ROW_BUFFER_SIZE);

    for (int rep = 0; rep < REPEAT; rep++) {
        // First copy operation timing, with cache flushed beforehand
        for (size_t i = 0; i < ROW_BUFFER_SIZE; i += 64) {
            clflush(buffer + i);
            clflush(buffer_copy + i);
        }
        asm volatile("mfence");

        start = rdtsc();
        memcpy(buffer_copy, buffer, ROW_BUFFER_SIZE);
        end = rdtsc();
        total_first += (end - start);

        // Second copy operation timing, no flush in between to test row buffer hit
        start = rdtsc();
        memcpy(buffer_copy, buffer, ROW_BUFFER_SIZE);
        end = rdtsc();
        total_second += (end - start);
    }

    printf("Average cycles first copy (cold row buffer): %lu\n", total_first / REPEAT);
    printf("Average cycles second copy (potential row buffer hit): %lu\n", total_second / REPEAT);

    free(buffer);
    free(buffer_copy);
}

int main(int ac, char **av) {
    printf("Testing DRAM row buffer policy with buffer size: %d bytes\n", ROW_BUFFER_SIZE);
    memtest();
    return 0;
}
