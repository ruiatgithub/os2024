#ifndef __MMU_H__
#define __MMU_H__

#include "prt_typedef.h"

#define CACHE_POS            0x2
#define CACHE_ENABLE         0x4
#define CACHE_MASK           0x7

#define MT_DEVICE_NGNRNE     0
#define MT_DEVICE_NGNRE      1
#define MT_DEVICE_GRE        2
#define MT_NORMAL_NC         3
#define MT_NORMAL            4

#define MEMORY_ATTRIBUTES    ((0x00 << (MT_DEVICE_NGNRNE * 8)) | \
                              (0x04 << (MT_DEVICE_NGNRE * 8))  | \
                              (0x0c << (MT_DEVICE_GRE * 8))    | \
                              (0x44 << (MT_NORMAL_NC * 8))     | \
                              (0xffUL << (MT_NORMAL * 8)))
// https://developer.arm.com/documentation/den0024/a/The-Memory-Management-Unit/Translation-tables-in-ARMv8-A/AArch64-descriptor-format
#define PTE_TYPE_FAULT       (0 << 0)
#define PTE_TYPE_BLOCK       (1 << 0)
#define PTE_TYPE_VALID       (1 << 0)
#define PTE_TYPE_MASK        (3 << 0)
#define PTE_TYPE_PAGE        (3 << 0)
#define PTE_TYPE_TABLE       (3 << 0)

#define PTE_TABLE_PXN        (1UL << 59)
#define PTE_TABLE_XN         (1UL << 60)
#define PTE_TABLE_AP         (1UL << 61)
#define PTE_TABLE_NS         (1UL << 63)

#define PTE_BLOCK_MEMTYPE(x) ((x) << 2)
#define PTE_BLOCK_NS         (1 << 5)
#define PTE_BLOCK_AP_R       (2 << 6)
#define PTE_BLOCK_AP_RW      (0 << 6)
#define PTE_BLOCK_NON_SHARE  (0 << 8)
#define PTE_BLOCK_OUTER_SHARE (2 << 8)
#define PTE_BLOCK_INNER_SHARE (3 << 8)
#define PTE_BLOCK_AF         (1 << 10)
#define PTE_BLOCK_NG         (1 << 11)
#define PTE_BLOCK_PXN        (1UL << 53)
#define PTE_BLOCK_UXN        (1UL << 54)

#define PMD_ATTRINDX(t)      ((t) << 2)
#define PMD_ATTRINDX_MASK    (7 << 2)
#define PMD_ATTRMASK         (PTE_BLOCK_PXN     | \
                              PTE_BLOCK_UXN     | \
                              PMD_ATTRINDX_MASK | \
                              PTE_TYPE_VALID)

#define TCR_IPS(x)           ((x) << 32)    // Intermediate Physical Address Size
#define TCR_T0SZ(x)          ((64 - (x)) << 0)
#define TCR_IRGN_NC          (0 << 8)
#define TCR_IRGN_WBWA        (1 << 8)
#define TCR_IRGN_WT          (2 << 8)
#define TCR_IRGN_WBNWA       (3 << 8)
#define TCR_IRGN_MASK        (3 << 8)
#define TCR_ORGN_NC          (0 << 10)
#define TCR_ORGN_WBWA        (1 << 10)
#define TCR_ORGN_WT          (2 << 10)
#define TCR_ORGN_WBNWA       (3 << 10)
#define TCR_ORGN_MASK        (3 << 10)
#define TCR_SHARED_NON       (0 << 12)
#define TCR_SHARED_OUTER     (2 << 12)
#define TCR_SHARED_INNER     (3 << 12)
#define TCR_TG0_4K           (0 << 14)
#define TCR_TG0_64K          (1 << 14)
#define TCR_TG0_16K          (2 << 14)
#define TCR_EPD1_DISABLE     (1 << 23)

#define TCR_EL1_RSVD         (1UL << 35)    //原定义错误
#define TCR_EL2_RSVD         (1UL << 31 | 1UL << 23)
#define TCR_EL3_RSVD         (1UL << 31 | 1UL << 23)

#define MAX_PTE_ENTRIES_4K   512
#define MAX_PTE_ENTRIES_64K  8192

#define PTE_TABLE_ADDR_MARK_4K  0x0000FFFFFFFFF000ULL
#define PTE_TABLE_ADDR_MARK_64K 0x0000FFFFFFFF0000ULL


#define MMU_ATTR_DEVICE_NGNRNE    (PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE))
#define MMU_ATTR_DEVICE           (PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRE))
#define MMU_ATTR_UNCACHE_UNSHARE  (PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) | PTE_BLOCK_NON_SHARE)
#define MMU_ATTR_UNCACHE_SHARE    (PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) | PTE_BLOCK_INNER_SHARE)
#define MMU_ATTR_CACHE_UNSHARE    (PTE_BLOCK_MEMTYPE(MT_NORMAL)    | PTE_BLOCK_NON_SHARE)
#define MMU_ATTR_CACHE_SHARE      (PTE_BLOCK_MEMTYPE(MT_NORMAL)    | PTE_BLOCK_INNER_SHARE)
#define MMU_ATTR_MASK             0X31CULL

#define MMU_ACCESS_NONE           (PTE_BLOCK_AP_RW)
#define MMU_ACCESS_R              (PTE_BLOCK_AF | PTE_BLOCK_UXN | PTE_BLOCK_PXN | PTE_BLOCK_AP_R)
#define MMU_ACCESS_RW             (PTE_BLOCK_AF | PTE_BLOCK_UXN | PTE_BLOCK_PXN | PTE_BLOCK_AP_RW)
#define MMU_ACCESS_RWX            (PTE_BLOCK_AF | PTE_BLOCK_AP_RW)
#define MMU_ACCESS_RX             (PTE_BLOCK_AF | PTE_BLOCK_AP_R)
#define MMU_ACCESS_MASK           0X600000000004C0ULL

#define MMU_GRANULE_4K            0
#define MMU_GRANULE_64K           1

#define MAX(a, b)                 (((a) > (b)) ? (a) : (b))
#define MIN(a, b)                 (((a) < (b)) ? (a) : (b))

#define CR_M                      (1 << 0)  // MMU enable
#define CR_A                      (1 << 1)  // Alignment check enable
#define CR_C                      (1 << 2)  // Cacheability control
#define CR_SA                     (1 << 3)  // SP(stack pointer) Alignment check enable
#define CR_I                      (1 << 12)
#define CR_WXN                    (1 << 19)
#define CR_EE                     (1 << 25)

#define SPSR_EL_END_LE            (0 << 9)
#define SPSR_EL_DEBUG_MASK        (1 << 9)
#define SPSR_EL_ASYN_MASK         (1 << 8)
#define SPSR_EL_SERR_MASK         (1 << 8)
#define SPSR_EL_IRQ_MASK          (1 << 7)
#define SPSR_EL_FIQ_MASK          (1 << 6)
#define SPSR_EL_T_A32             (0 << 5)
#define SPSR_EL_M_AARCH64         (0 << 4)
#define SPSR_EL_M_AARCH32         (1 << 4)
#define SPSR_EL_M_SVC             (0x3)
#define SPSR_EL_M_HYP             (0xa)
#define SPSR_EL_M_EL1H            (5)
#define SPSR_EL_M_EL2H            (9)

#define CPUECTLR_EL1_L1PCTL_MASK  (7 << 13)
#define CPUECTLR_EL1_L3PCTL_MASK  (7 << 10)

typedef enum {
    MMU_LEVEL_0 = 0,
    MMU_LEVEL_1,
    MMU_LEVEL_2,
    MMU_LEVEL_3,
    MMU_LEVEL_MAX
} mmu_level_e;

typedef enum {
    MMU_PHY_ADDR_LEVEL_0 = 0,
    MMU_PHY_ADDR_LEVEL_1,
    MMU_PHY_ADDR_LEVEL_2,
    MMU_PHY_ADDR_LEVEL_3,
    MMU_PHY_ADDR_LEVEL_4,
    MMU_PHY_ADDR_LEVEL_5
} mmu_physical_addr_size_e;

typedef enum {
    MMU_BITS_9 = 9,
    MMU_BITS_12 = 12,
    MMU_BITS_13 = 13,
    MMU_BITS_16 = 16,
    MMU_BITS_32 = 32,
    MMU_BITS_36 = 36,
    MMU_BITS_39 = 39,
    MMU_BITS_40 = 40,
    MMU_BITS_42 = 42,
    MMU_BITS_44 = 44,
    MMU_BITS_48 = 48,
} mmu_bits_e;

typedef struct {
    U64 tlb_addr;
    U64 tlb_size;
    U64 tlb_fillptr;
    U32 granule;
    U32 start_level;
    U32 va_bits;
} mmu_ctrl_s;

typedef struct {
    U64 virt;
    U64 phys;
    U64 size;
    U64 max_level;
    U64 attrs;
} mmu_mmap_region_s;

static inline unsigned long get_sctlr(void)
{
    unsigned long val;
    
    __asm__ __volatile__("mrs %0, sctlr_el1" : "=r" (val) : : "cc");

    return val;
}

static inline unsigned long get_cpuectr(void)
{
    unsigned long val;

    __asm__ __volatile__("mrs %0, S3_1_C15_C2_1" : "=r" (val) : : "cc");

    return val;
}

static inline void set_sctlr(unsigned long val)
{
    __asm__ __volatile__("dsb sy");
    __asm__ __volatile__("msr sctlr_el1, %0" : : "r" (val) : "cc");
    __asm__ __volatile__("dsb sy");
    __asm__ __volatile__("isb");
}

extern S32 mmu_init(void);

#endif
