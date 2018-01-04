#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
// Hashes
#include "jhash.h"
#include "xxhash.h"
#include "cityhash.h"


#define PAGE_SIZE (1024)
uint64_t iter = 1024;
static uint32_t PAGE[PAGE_SIZE];

void run_test(size_t len) {
        uint64_t i;
        uint32_t hash = 0;
        uint64_t hash64 = 0;
        clock_t start, end;

        printf("- - -\n");
        printf("input size: %lu, loop count: %lu\n", len, iter);

        if (len % 4 == 0) {
                start = clock()*1000000/CLOCKS_PER_SEC;
                for (i = 0; i < iter; i++) {
                        hash = jhash2(&PAGE[hash%(PAGE_SIZE/2)], len/4, 17);
                }
                end = clock()*1000000/CLOCKS_PER_SEC;

                printf("jhash2:   0x%" PRIx32 "\t\ttime: %6lu ms, th: %.2f MiB/s\n", hash, (end - start)/1000, len*iter*1.0/(end - start));
        } else {
                start = clock()*1000000/CLOCKS_PER_SEC;
                for (i = 0; i < iter; i++) {
                        hash = jhash(&PAGE[hash%(PAGE_SIZE/2)], len, 17);
                }
                end = clock()*1000000/CLOCKS_PER_SEC;

                printf("jhash:    0x%" PRIx32 "\t\ttime: %6lu ms, th: %.2f MiB/s\n", hash, (end - start)/1000, len*iter*1.0/(end - start));
        }

        start = clock()*1000000/CLOCKS_PER_SEC;
        for (i = 0; i < iter; i++) {
                hash = cityhash32((uint8_t *) &PAGE[hash%(PAGE_SIZE/2)], len);
        }
        end = clock()*1000000/CLOCKS_PER_SEC;

        printf("City32:   0x%" PRIx32 "\t\ttime: %6lu ms, th: %.2f MiB/s\n", hash, (end - start)/1000, len*iter*1.0/(end - start));

        start = clock()*1000000/CLOCKS_PER_SEC;
        for (i = 0; i < iter; i++) {
                hash = xxh32(&PAGE[hash%(PAGE_SIZE/2)], len, 17);
        }
        end = clock()*1000000/CLOCKS_PER_SEC;

        printf("xxhash32: 0x%" PRIx32 "\t\ttime: %6lu ms, th: %.2f MiB/s\n", hash, (end - start)/1000, len*iter*1.0/(end - start));

        start = clock()*1000000/CLOCKS_PER_SEC;
        for (i = 0; i < iter; i++) {
                hash64 = xxh64(&PAGE[hash%(PAGE_SIZE/2)], len, 17);
        }
        end = clock()*1000000/CLOCKS_PER_SEC;

        printf("xxhash64: 0x%" PRIx64 "\ttime: %6lu ms, th: %.2f MiB/s\n", hash64, (end - start)/1000, len*iter*1.0/(end - start));
}

int main() {
        uint64_t i;
        uint32_t input_lenths[11] = {
                3, 4, 8, 11,
                12, 16, 17, 33,
                36, 64, 67
        };

        srand(time(NULL));
        iter *= 1024*256;

        for (i = 0; i < sizeof(PAGE)/4; i++)
                PAGE[i] = rand();

        for (i = 0; i < 11; i++) {
                run_test(input_lenths[i]);
        }
        return 0;
}
