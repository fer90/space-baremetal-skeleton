.section .text.init
.global _start
.type _start, @function

_start:
    # Set up stack
    la sp, stack_top

    # Zero .bss section
    la t0, __bss_start
    la t1, __bss_end
1:  bge t0, t1, 2f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 1b
2:

    call main

    # Infinite loop (should never reach here in normal operation)
1:  wfi
    j 1b

# Stack section
.section .bss
.align 8
.global stack_low
stack_low:
    .skip 4096   # 4KB stack
.global stack_top
stack_top:
