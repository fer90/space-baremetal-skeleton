/* Improved linker script for RISC-V bare metal - Space Skeleton */
ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    .text.init : {
        *(.text.init)
    }

    .text : {
        *(.text*)
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