#pragma once

#include "stddef.h"
#include "stdio.h"
#include "inttypes.h"

/** Bit mask stuff **/
static const size_t BITS_PER_LONG = 8 * sizeof(long);
static const size_t BITS_PER_LONG_LONG = 8 * sizeof(long long);

#define UL(x) x##UL
#define ULL(x) x##ULL

#define GENMASK(h, l)                  \
    (((~UL(0)) - (UL(1) << (l)) + 1) & \
     (~UL(0) >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l)                \
    (((~ULL(0)) - (ULL(1) << (l)) + 1) & \
     (~ULL(0) >> (BITS_PER_LONG_LONG - 1 - (h))))

#define print_bits(x)                                      \
    do {                                                   \
        typeof(x) a__ = (x);                               \
        char *p__ = (char *)&a__ + sizeof(x) - 1;          \
        size_t bytes__ = sizeof(x);                        \
        printf(#x ": ");                                   \
        while (bytes__--) {                                \
            char bits__ = 8;                               \
            while (bits__--)                               \
                putchar(*p__ & (1ULL << bits__) ? '1' : '0'); \
            p__--;                                         \
        }                                                  \
        putchar('\n');                                     \
    } while (0)

// IMP_PF_CTRL_EL1 : S3_0_C11_C4_0
// set bit 62-63 makes IMP_PF_STREAM_DETECT_CTRL_EL0 accessible from EL0
//
// IMP_PF_STREAM_DETECT_CTRL_EL0: S3_3_C11_C4_0
// [27:24] L1_DISTRW
// The distance of the hardware prefetch to L1 cache is specified.
// The prefetch distance for L1 is calculated as (L1_DIST * 256B).
// When L1_DIST = 0, the hardware prefetch for L1 operates by the
// default distance.
// [19:16] L2_DISTRW
// The distance of the hardware prefetch to L2 cache is specified.
// The prefetch distance for L2 is calculated as (L2_DIST * 1KB).
// When L2_DIST = 0, the hardware prefetch for L2 operates by the
// default distance.

#define A64FX_ENABLE_PF_CTRL() \
{\
    uint64_t val = GENMASK_ULL(63, 62); \
__asm__ volatile(\
            "msr S3_0_C11_C4_0,%[val]"\
            :\
            : [val] "r" (val)\
            :\
            );\
}

#define A64FX_DISABLE_PF_CTRL() \
{\
    uint64_t val = 0ULL; \
__asm__ volatile(\
            "msr S3_0_C11_C4_0,%[val]"\
            :\
            : [val] "r" (val)\
            :\
            );\
}


// The distance of the hardware prefetch to L1 cache is specified.
// The prefetch distance for L1 is calculated as (L1_DIST * 256B).
// When L1_DIST = 0, the hardware prefetch for L1 operates by the
// default distance.

// read the old value of the register into @orig and 
// set the l1 prefetch distance to @dist
#define A64FX_SET_PF_DST_L1(orig, dist) \
{\
    __asm__ volatile(\
            "mrs %["#orig"],S3_3_C11_C4_0"\
            : [orig] "=r" (orig)\
            : \
            :\
            ); \
    uint64_t distu64 = dist; \
    uint64_t val = orig; \
    uint64_t mask = ~GENMASK_ULL(27, 24); \
    val = val & mask; \
    val = val | (distu64 << 24) | (1ULL << 63); \
    __asm__ volatile(\
            "msr S3_3_C11_C4_0,%[val]"\
            :\
            : [val] "r" (val)\
            :\
            );\
}

// The distance of the hardware prefetch to L2 cache is specified.
// The prefetch distance for L2 is calculated as (L2_DIST * 1KB).
// When L2_DIST = 0, the hardware prefetch for L2 operates by the
// default distance.

// read the old value of the register into @orig and 
// set the l1 prefetch distance to @dist
#define A64FX_SET_PF_DST_L2(orig, dist) \
{\
    __asm__ volatile(\
            "mrs %["#orig"],S3_3_C11_C4_0"\
            : [orig] "=r" (orig)\
            : \
            :\
            ); \
    uint64_t distu64 = dist; \
    uint64_t val = orig; \
    uint64_t mask = ~GENMASK_ULL(19, 16); \
    val = val & mask; \
    val = val | (distu64 << 16) | (1ULL << 63); \
    __asm__ volatile(\
            "msr S3_3_C11_C4_0,%[val]"\
            :\
            : [val] "r" (val)\
            :\
            );\
}

// reads the register value
#define A64FX_READ_PF_DST(orig) \
{\
    __asm__ volatile(\
            "mrs %["#orig"],S3_3_C11_C4_0"\
            : [orig] "=r" (orig)\
            : \
            :\
            ); \
}

// sets the register value
#define A64FX_WRITE_PF_DST(regval) \
{\
    uint64_t val = regval; \
    __asm__ volatile(\
            "msr S3_3_C11_C4_0,%[val]"\
            :\
            : [val] "r" (val)\
            :\
            );\
}

#if 0
// test code and example usage
int main() {
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

#endif