#include "a64fx_prf_dst.h"

#include <stdio.h>

#define print_bits(x)                                                                    \
    do {                                                                                 \
        typeof(x) a__     = (x);                                                         \
        char     *p__     = (char *)&a__ + sizeof(x) - 1;                                \
        size_t    bytes__ = sizeof(x);                                                   \
        printf(#x ": ");                                                                 \
        while (bytes__--) {                                                              \
            char bits__ = 8;                                                             \
            while (bits__--)                                                             \
                putchar(*p__ & (1ULL << bits__) ? '1' : '0');                            \
            p__--;                                                                       \
        }                                                                                \
        putchar('\n');                                                                   \
    } while (0)

// test code and example usage
int main()
{
    printf("old register values:\n");
#pragma omp parallel
    {
        // A64FX_ENABLE_PF_CTRL(); // only accessible from EL1

        uint64_t orig;
        A64FX_READ_PF_DST(orig); // first read the original value
#pragma omp critical
        print_bits(orig); // should print only zeroes

        uint64_t tmp;
        A64FX_SET_PF_DST_L1(tmp, 3);
        A64FX_SET_PF_DST_L2(tmp, 7);

        A64FX_READ_PF_DST(tmp);

#pragma omp barrier
#pragma omp single
        printf("new register values:\n");
#pragma omp barrier

#pragma omp critical
        print_bits(tmp); // should print also ones

        A64FX_WRITE_PF_DST(orig);
        A64FX_READ_PF_DST(tmp); // first read the original value

#pragma omp barrier
#pragma omp single
        printf("reset register values:\n");
#pragma omp barrier

#pragma omp critical
        print_bits(tmp); // should print only zeroes

        // A64FX_DISABLE_PF_CTRL(); // only accessible from EL1
    }
}
