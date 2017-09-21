#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>

// Hashes
#include "jhash.h"
#include "xxhash.h"

int main() {
        uint8_t PAGE[4*1024];
        uint32_t hash;
        uint64_t hash64;
        uint64_t iter = 1024;
        uint64_t i;
        srand(time(NULL));
        clock_t start, end;

        iter *= 1024*1024*4/sizeof(PAGE);

        for (i = 0; i < sizeof(PAGE)/4; i++)
                PAGE[i] = rand();

        printf("PAGE_SIZE: %lu, loop count: %lu\n", sizeof(PAGE), iter);

        start = clock()*1000000/CLOCKS_PER_SEC;
        for (i = 0; i < iter; i++) {
                hash = jhash2((uint32_t *) PAGE, sizeof(PAGE)/4, 17);
        }
        end = clock()*1000000/CLOCKS_PER_SEC;

        printf("jhash2:   0x%" PRIx32 "\t\ttime: %lu ms,\tth: %f MiB/s\n", hash, (end - start)/1000, sizeof(PAGE)*iter*1.0/(end - start));

        start = clock()*1000000/CLOCKS_PER_SEC;
        for (i = 0; i < iter; i++) {
                hash = xxh32(PAGE, sizeof(PAGE), 17);
        }
        end = clock()*1000000/CLOCKS_PER_SEC;


        printf("xxhash32: 0x%" PRIx32 "\t\ttime: %lu ms,\tth: %f MiB/s\n", hash, (end - start)/1000, sizeof(PAGE)*iter*1.0/(end - start));

        start = clock()*1000000/CLOCKS_PER_SEC;
        for (i = 0; i < iter; i++) {
                hash64 = xxh64(PAGE, sizeof(PAGE), 17);
        }
        end = clock()*1000000/CLOCKS_PER_SEC;

        printf("xxhash64: 0x%" PRIx64 "\ttime: %lu ms,\tth: %f MiB/s\n", hash64, (end - start)/1000, sizeof(PAGE)*iter*1.0/(end - start));

        return 0;
}
