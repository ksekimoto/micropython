    .file   "gchelper.s"
    .text

    .align  4
# uint gc_helper_get_regs_and_sp(r1=uint regs[17])
    .global _gc_helper_get_regs_and_sp
    .type   _gc_helper_get_regs_and_sp, @function
_gc_helper_get_regs_and_sp:
    # store regs into given array
    push    r2
    mov.l   r0, 00[r1]
    mov.l   r1, 04[r1]
    mov.l   r2, 08[r1]
    mov.l   r3, 12[r1]
    mov.l   r4, 16[r1]
    mov.l   r5, 20[r1]
    mov.l   r6, 24[r1]
    mov.l   r7, 28[r1]
    mov.l   r8, 32[r1]
    mov.l   r9, 36[r1]
    mov.l   r10, 40[r1]
    mov.l   r11, 44[r1]
    mov.l   r12, 48[r1]
    mov.l   r13, 52[r1]
    mov.l   r14, 56[r1]
    mov.l   r15, 60[r1]
    mvfc    psw, r2
    mov.l   r2, 64[r1]
    pop     r2
    mov.l   r0, r1
    # return the sp
    rts

    .size   _gc_helper_get_regs_and_sp, .-_gc_helper_get_regs_and_sp
