/*
 * rpi3.ld - Linker script para Raspberry Pi 3
 */

MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = 0x3F000000
}

SECTIONS
{
    .text.boot : {
        *(.text.boot)
    } > ram
    
    .text : {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
    } > ram
    
    .data : {
        . = ALIGN(4);
        *(.data)
        *(.data.*)
        . = ALIGN(4);
    } > ram
    
    .bss : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > ram
    
    .heap : {
        . = ALIGN(4);
        __heap_start = .;
        . = . + 0x100000; /* 1MB heap */
        __heap_end = .;
    } > ram
    
    .stack : {
        . = ALIGN(8);
        . = . + 0x10000; /* 64KB stack */
        __stack_top = .;
    } > ram
}