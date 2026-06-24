/* Improved linker script for RISC-V bare metal - Space Skeleton */
ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    .text.init : {
        *(.text.init)
    }

    .text : {
        __critical_text_start = .;
        *(.text.critical .text.critical.*)
        __critical_text_end = .;
        *(.text .text.*)
        *(.gnu.linkonce.t*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
}