/* demos/hi3093/build/hi3093.ld */
ENTRY(__text_start)

_stack_size = 0x10000;
_heap_size = 0x10000;

MEMORY
{
    IMU_SRAM (rwx) : ORIGIN = 0x40000000, LENGTH = 0x800000
    MMU_MEM (rwx) : ORIGIN = 0x40800000, LENGTH = 0x800000
}

SECTIONS
{
    text_start = .;
    .start_bspinit :
    {
        __text_start = .;
        KEEP(*(.text.bspinit))
    } > IMU_SRAM
    
    .start_text :
    {
        KEEP(*(.text.startup))
    } > IMU_SRAM
    
    .text :
    {
        *(.text)
        *(.text.*)
        *(*.text)
        . = ALIGN(8);
        __text_end = .;
    } > IMU_SRAM
    . = ALIGN(8);
    data_copy_start = .;
    
    .rodata :
    {
        . = ALIGN(8);
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    } > IMU_SRAM
    
    .eh_frame :
    {
        . = ALIGN(8);
        __os_unwind_table_start = .;
        *(.eh_frame)
        __os_unwind_table_end = .;
    } > IMU_SRAM
    
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE (__HEAP_INIT = .);
        . = . + _heap_size;
        . = ALIGN(8);
        PROVIDE (__HEAP_END = .);
    } > IMU_SRAM
    
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE (__os_sys_sp_start = .);
        . = . + _stack_size;
        . = ALIGN(8);
        PROVIDE (__os_sys_sp_end = .);
    } > IMU_SRAM
    end = .;
    
    .percpu.data :
    {
        __os_per_cpu_start = .;
        *(.os.percpu.data)
        __os_per_cpu_end = .;
        LONG (ALIGNOF(.percpu.data))
    } > IMU_SRAM
    
    .data :
    {
        . = ALIGN(8);
        __data_start = .;
        *(.data)
        *(.data.*)
        . = ALIGN(8);
        __os_text_start = .;
        QUAD(__text_start)
        QUAD(__text_end)
        __os_text_end = .;
        __data_end = .;
    } > IMU_SRAM
    
    .llt.bss :
    {
        _llt_bss_start = .;
        *__code_measure_stub*.o(.bss)
        *__code_measure_stub*.o(.bss.*)
        _llt_bss_end = .;
    } > IMU_SRAM
    
    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
    } > IMU_SRAM
    
    .mmu.table.base :
    {
        PROVIDE (g_mmu_page_begin = .);
        PROVIDE (g_mmu_page_end = g_mmu_page_begin + 0x8000);
    } > MMU_MEM
}
