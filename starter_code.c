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
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return a | ((uint64_t)d << 32);
}

char lineBuffer[64];
long int rep;

void memtest(FILE *f, int power) {
    uint64_t start, end, clock;
    char *lineBuffer = (char *)malloc(64);
    char *lineBufferCopy = (char *)malloc(64);
    for (int i = 0; i < 64; i++) {
        lineBuffer[i] = '1';
    }
    clock = 0;


    for (rep = 0; rep < REPEAT; rep++) {
        start = rdtsc();
        memcpy(lineBufferCopy, lineBuffer, 64);
        end = rdtsc();
        clflush(lineBuffer);
        clflush(lineBufferCopy);
        clock = clock + (end - start);
        //printf("%lu ticks to copy 64B\n", (end - start));
        int ticks = (int)(end - start); 
        fprintf(f, "%d", ticks);
        if(rep < (REPEAT - 1)){
            fputc(',', f);
        } 
         
    }
    fputc('\n', f);
    printf(" took %lu ticks total\n", clock);

    
    
}

int main(int ac, char **av) {
    printf("--------------------------------\n");
    FILE *f = fopen("results.csv", "w");
    int a[] = {6,7,8,9,10,11,12,13,14,15,16,20,21};
    for(int i = 0; i < sizeof(a)/sizeof(int); i++){
        printf("%d\n", i);
        int j = a[i];
        memtest(f,j);
    }
    fclose(f);
    return 0;
}

