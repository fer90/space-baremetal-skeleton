/* Improved linker script for RISC-V bare metal - Space Skeleton */
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);  /* PF_R | PF_X — read + execute */
    data PT_LOAD FLAGS(6);  /* PF_R | PF_W — read + write */
}

SECTIONS
{
    . = 0x80000000;
    __image_integrity_start = .;

    .text.init : {
        *(.text.init)
    } :text

    .text : {
        __critical_text_start = .;
        *(.text.critical .text.critical.*)
        __critical_text_end = .;
        *(.text .text.*)
        *(.gnu.linkonce.t*)
    } :text

    .rodata : {
        *(.rodata*)
        . = ALIGN(4);
        __image_crc_slot = .;
        KEEP(*(.image_crc))
        __image_integrity_end = .;
    } :text

    .data : {
        *(.data)
    } :data

    .bss : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } :data
}