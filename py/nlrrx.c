/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/mpstate.h"

#if MICROPY_NLR_RX

#undef nlr_push

// We only need the functions here if we are on renesas rx, and we are not
// using setjmp/longjmp.
//
// For reference, rx callee save regs are:
//      r0-r15,...

__attribute__((naked)) unsigned int nlr_push(nlr_buf_t *nlr) {

    __asm volatile (
    "mov.l    r0, 08[r1]        \n" // store r0 into nlr_buf
    "mov.l    r1, 12[r1]        \n" // store r1 into nlr_buf
    "mov.l    r2, 16[r1]        \n" // store r2 into nlr_buf
    "mov.l    r3, 20[r1]        \n" // store r3 into nlr_buf
    "mov.l    r4, 24[r1]        \n" // store r4 into nlr_buf
    "mov.l    r5, 28[r1]        \n" // store r5 into nlr_buf
    "mov.l    r6, 32[r1]        \n" // store r6 into nlr_buf
    "mov.l    r7, 36[r1]        \n" // store r7 into nlr_buf
    "mov.l    r8, 40[r1]        \n" // store r8 into nlr_buf
    "mov.l    r9, 44[r1]        \n" // store r9 into nlr_buf
    "mov.l    r10, 48[r1]       \n" // store r10 into nlr_buf
    "mov.l    r11, 52[r1]       \n" // store r11 into nlr_buf
    "mov.l    r12, 56[r1]       \n" // store r12 into nlr_buf
    "mov.l    r13, 60[r1]       \n" // store r13 into nlr_buf
    "mov.l    r14, 64[r1]       \n" // store r14 into nlr_buf
    "mov.l    r15, 68[r1]       \n" // store r15 into nlr_buf
    "mov.l    [r0],72[r1]       \n" // store return address into nlr_buf
    "mvfc     psw, r2           \n" //
    "mov.l    r2, 76[r1]        \n" // store psw into nlr_buf
    "mov.l    #nlr_push_tail_var, r2 \n"    // do the rest in C
    "mov.l    [r2], r2          \n" // store jmp addr into r2
    "jmp      r2                \n" // do the rest in C
    ".align 2                   \n"
    "nlr_push_tail_var: .word _nlr_push_tail \n"
    );

    #if !defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8))
    // Older versions of gcc give an error when naked functions don't return a value
    // Additionally exclude Clang as it also defines __GNUC__ but doesn't need this statement
    return 0;
    #endif
}

NORETURN void nlr_jump(void *val) {
    MP_NLR_JUMP_HEAD(val, top)

    __asm volatile (
    "mov.l  r1, %0              \n" // r0 points to nlr_buf
    "mov.l  00[r1], r0          \n" // load r0 from nlr_buf
    "mov.l  04[r1], r1          \n" // load r1 from nlr_buf
    "mov.l  12[r1], r3          \n" // load r3 from nlr_buf
    "mov.l  16[r1], r4          \n" // load r4 from nlr_buf
    "mov.l  20[r1], r5          \n" // load r5 from nlr_buf
    "mov.l  24[r1], r6          \n" // load r6 from nlr_buf
    "mov.l  28[r1], r7          \n" // load r7 from nlr_buf
    "mov.l  32[r1], r8          \n" // load r8 from nlr_buf
    "mov.l  36[r1], r9          \n" // load r9  from nlr_buf
    "mov.l  40[r1], r10         \n" // load r10 from nlr_buf
    "mov.l  44[r1], r11         \n" // load r11 from nlr_buf
    "mov.l  48[r1], r12         \n" // load r12 from nlr_buf
    "mov.l  52[r1], r13         \n" // load r13 from nlr_buf
    "mov.l  56[r1], r14         \n" // load r14 from nlr_buf
    "mov.l  60[r1], r15         \n" // load r15 from nlr_buf
    "mov.l  68[r1], r2          \n" // load psw from nlr_buf
    "mvtc   r2, psw             \n" //
    "mov.l  64[r1], r2          \n" // load r2 from nlr_buf
    "mov.l  r2, [r0]            \n" // load r2 from nlr_buf
    "mov.l  08[r1], r2          \n" // load r2 from nlr_buf
    "rts                        \n" // return
    :                               // output operands
    : "r"(top)                      // input operands
    :                               // clobbered registers
    );

    #if defined(__GNUC__)
    __builtin_unreachable();
    #else
    for (;;); // needed to silence compiler warning
    #endif
}

#endif // MICROPY_NLR_RX
