.global os_asm_invalidate_dcache_all
.global os_asm_flush_dcache_all
.global os_asm_clean_dcache_all
.global os_asm_invalidate_icache_all
.global os_asm_invalidate_tlb_all

    .type os_asm_dcache_level, "function"
os_asm_dcache_level:
    lsl    x12, x0, #1
    msr    csselr_el1, x12
    isb
    mrs    x6, ccsidr_el1
    and    x2, x6, #7
    add    x2, x2, #4
    mov    x3, #0x3ff
    and    x3, x3, x6, lsr #3
    clz    w5, w3
    mov    x4, #0x7fff
    and    x4, x4, x6, lsr #13
    
loop_set:
    mov    x6, x3
loop_way:
    lsl    x7, x6, x5
    orr    x9, x12, x7
    lsl    x7, x4, x2
    orr    x9, x9, x7
    tbz    w1, #1, 3f
    dc     csw, x9
    b      2f
3:  tbz    w1, #0, 1f
    dc     isw, x9
    b      2f
1:  dc     cisw, x9
2:  subs   x6, x6, #1
    b.ge   loop_way
    subs   x4, x4, #1
    b.ge   loop_set
    
    ret
    
    .type os_asm_dcache_all, "function"
os_asm_dcache_all:
    mov    x1, x0
    dsb    sy
    mrs    x10, clidr_el1
    lsr    x11, x10, #24
    and    x11, x11, #0x7
    cbz    x11, finished
    mov    x15, x30
    mov    x0, #0
    
loop_level:
    lsl    x12, x0, #1
    add    x12, x12, x0
    lsr    x12, x10, x12
    and    x12, x12, #7
    cmp    x12, #2
    b.lt   skip
    bl     os_asm_dcache_level
skip:
    add    x0, x0, #1
    cmp    x11, x0
    b.gt   loop_level
    
    mov    x0, #0
    msr    csselr_el1, x0
    dsb    sy
    isb
    mov    x30, x15
    
finished:
    ret

    .type os_asm_invalidate_dcache_all, "function"
os_asm_invalidate_dcache_all:
    mov    x0, #0x1
    b      os_asm_dcache_all

    .type os_asm_flush_dcache_all, "function"
os_asm_flush_dcache_all:
    mov    x0, #0x0
    b      os_asm_dcache_all

    .type os_asm_clean_dcache_all, "function"
os_asm_clean_dcache_all:
    mov    x0, #0x2
    b      os_asm_dcache_all

    .type os_asm_invalidate_icache_all, "function"
os_asm_invalidate_icache_all:
    ic     ialluis
    isb    sy
    ret

    .type os_asm_invalidate_tlb_all, "function"
os_asm_invalidate_tlb_all:
    tlbi   vmalle1
    dsb    sy
    isb
    ret
