#pragma once

#include <inttypes.h>
#include <stddef.h>

/** Bit mask stuff **/
static const size_t BITS_PER_LONG      = 8 * sizeof(long);
static const size_t BITS_PER_LONG_LONG = 8 * sizeof(long long);

#define UL(x)  x##UL
#define ULL(x) x##ULL

#define GENMASK(h, l)                                                                    \
    (((~UL(0)) - (UL(1) << (l)) + 1) & (~UL(0) >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l)                                                                \
    (((~ULL(0)) - (ULL(1) << (l)) + 1) & (~ULL(0) >> (BITS_PER_LONG_LONG - 1 - (h))))

#define ARM_READ_MRS(val_u64, reg_name)                                                  \
    __asm__ volatile("mrs %[" #val_u64 "]," #reg_name : [val_u64] "=r"(val_u64) : :);

#define ARM_WRITE_MSR(val_u64, reg_name)                                                 \
    __asm__ volatile("msr " #reg_name ",%[" #val_u64 "]" : : [val_u64] "r"(val_u64) :);

// IMP_PF_CTRL_EL1 : S3_0_C11_C4_0
// set bit 62-63 makes IMP_PF_STREAM_DETECT_CTRL_EL0 accessible from EL0

#define A64FX_ENABLE_PF_CTRL()                                                           \
    {                                                                                    \
        uint64_t val = GENMASK_ULL(63, 62);                                              \
        ARM_WRITE_MSR(val, S3_0_C11_C4_0);                                               \
    }

#define A64FX_DISABLE_PF_CTRL()                                                          \
    {                                                                                    \
        uint64_t val = 0ULL;                                                             \
        ARM_WRITE_MSR(val, S3_0_C11_C4_0);                                               \
    }

// IMP_PF_STREAM_DETECT_CTRL_EL0: S3_3_C11_C4_0
//
// [63] V RW
// 1: A value of IMP_PF_STREAM_DETECT_CTRL_EL0 is valid
// 0: It operates by a set value of Default.
//
// [27:24] L1_DISTRW
// The distance of the hardware prefetch to L1 cache is specified.
// The prefetch distance for L1 is calculated as (L1_DIST * 256B).
// When L1_DIST = 0, the hardware prefetch for L1 operates by the
// default distance.
//
// [19:16] L2_DISTRW
// The distance of the hardware prefetch to L2 cache is specified.
// The prefetch distance for L2 is calculated as (L2_DIST * 1KB).
// When L2_DIST = 0, the hardware prefetch for L2 operates by the
// default distance.

// read the old value of the register into @val_u64 and
// set the l1 prefetch distance to @dist
#define A64FX_SET_PF_DST_L1(val_u64, dist)                                               \
    {                                                                                    \
        ARM_READ_MRS(val_u64, S3_3_C11_C4_0);                                            \
        uint64_t distu64 = dist;                                                         \
        uint64_t val     = val_u64;                                                      \
        uint64_t mask    = ~GENMASK_ULL(27, 24);                                         \
        val              = val & mask;                                                   \
        val              = val | (distu64 << 24) | (1ULL << 63);                         \
        ARM_WRITE_MSR(val, S3_3_C11_C4_0);                                               \
    }

// read the old value of the register into @val_u64 and
// set the l2 prefetch distance to @dist
#define A64FX_SET_PF_DST_L2(val_u64, dist)                                               \
    {                                                                                    \
        ARM_READ_MRS(val_u64, S3_3_C11_C4_0);                                            \
        uint64_t distu64 = dist;                                                         \
        uint64_t val     = val_u64;                                                      \
        uint64_t mask    = ~GENMASK_ULL(19, 16);                                         \
        val              = val & mask;                                                   \
        val              = val | (distu64 << 16) | (1ULL << 63);                         \
        ARM_WRITE_MSR(val, S3_3_C11_C4_0);                                               \
    }

// reads the register value
#define A64FX_READ_PF_DST(val_u64)                                                       \
    {                                                                                    \
        ARM_READ_MRS(val_u64, S3_3_C11_C4_0);                                            \
    }

// sets the register value
#define A64FX_WRITE_PF_DST(regval)                                                       \
    {                                                                                    \
        uint64_t val = regval;                                                           \
        ARM_WRITE_MSR(val, S3_3_C11_C4_0);                                               \
    }
