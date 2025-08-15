/* 
 * Linker script for Raspberry Pi 3 bare metal
 */

ENTRY(_start)

MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = 0x37F8000
}

SECTIONS
{
    . = 0x8000;
    
    .text : {
        *(.text.boot)
        *(.text)
        *(.rodata)
        *(.rodata.*)
    } > ram
    
    .data : {
        *(.data)
    } > ram
    
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    } > ram
    
    . = ALIGN(8);
    . = . + 0x4000; /* 16k stack */
    stack_top = .;
}