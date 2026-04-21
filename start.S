.section .text.boot
.global _start

_start:
    /* load top of stack from linker symbol */
    la sp, _stack_top

    /* zero the BSS segment */
    la t0, _bss_start
    la t1, _bss_end
1:
    bge t0, t1, 2f
    sb zero, 0(t0)
    addi t0, t0, 1
    j 1b
2:
    /* call C main — never returns */
    call main

    /* halt loop in case main returns */
3:
    j 3b
