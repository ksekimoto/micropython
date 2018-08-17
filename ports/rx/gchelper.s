    .file   "gchelper.s"
    .text

    .align  4
    .global _gc_helper_get_regs_and_sp
    .type   _gc_helper_get_regs_and_sp, @function
_gc_helper_get_regs_and_sp:
    # store regs into given array
    mov.l   r4, [r1+]
    mov.l   r5, [r1+]
    mov.l   r6, [r1+]
    mov.l   r7, [r1+]
    mov.l   r8, [r1+]
    mov.l   r9, [r1+]
    mov.l   r10, [r1+]
    mov.l   r11, [r1+]
    mov.l   r12, [r1+]
    mov.l   r13, [r1+]
    mov.l   r14, [r1+]
    mov.l   r15, [r1+]
    # return the sp
    mvfc    usp, r1
    rts

    .size   _gc_helper_get_regs_and_sp, .-_gc_helper_get_regs_and_sp
