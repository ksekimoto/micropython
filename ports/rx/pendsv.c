/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2018 Kentaro Sekimoto
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

#include <stdlib.h>

#include "py/runtime.h"
#include "lib/utils/interrupt_char.h"
#include "pendsv.h"

// This variable is used to save the exception object between a ctrl-C and the
// PENDSV call that actually raises the exception.  It must be non-static
// otherwise gcc-5 optimises it away.  It can point to the heap but is not
// traced by GC.  This is okay because we only ever set it to
// mp_kbd_exception which is in the root-pointer set.
void *pendsv_object;

void pendsv_init(void) {
    // set PendSV interrupt at lowest priority
}

// Call this function to raise a pending exception during an interrupt.
// It will first try to raise the exception "softly" by setting the
// mp_pending_exception variable and hoping that the VM will notice it.
// If this function is called a second time (ie with the mp_pending_exception
// variable already set) then it will force the exception by using the hardware
// PENDSV feature.  This will wait until all interrupts are finished then raise
// the given exception object using nlr_jump in the context of the top-level
// thread.
void pendsv_kbd_intr(void) {
    if (MP_STATE_VM(mp_pending_exception) == MP_OBJ_NULL) {
        mp_keyboard_interrupt();
    } else {
        MP_STATE_VM(mp_pending_exception) = MP_OBJ_NULL;
        pendsv_object = &MP_STATE_VM(mp_kbd_exception);
        __asm volatile (
            "int    #1\n"
        );
    }
}

void __attribute__ ((naked)) INT_Excep_1(void) {
    // re-jig the stack so that when we return from this interrupt handler
    // it returns instead to nlr_jump with argument pendsv_object
    // note that stack has a different layout if DEBUG is enabled
    //
    // on entry to this (naked) function, stack has the following layout:
    //
    // normal interrupt
    // stack layout with DEBUG disabled:
    //   sp[1]: psw
    //   sp[0]: pc
    //
    // stack layout with DEBUG enabled:
    //   sp[3]: psw
    //   sp[2]: pc
    //   sp[1]: 0xfffffff9
    //   sp[0]: ?

#if MICROPY_PY_THREAD
#if 0
    __asm volatile (

    );
#endif
#else
    __asm volatile (
        //"mov.l  [r0], r1\n"
        //"mvtc   r1, psw\n"
        "mov.l  #nlr_jump_ptr, r1\n"
        "mov.l  [r1], [r0]\n"
        "mov.l  #pendsv_object_ptr, r1\n"
        "mov.l  [r1], r1\n"                 /* r1: &pendsv_object*/
        "mov.l  [r1], r1\n"                 /* r1: pendsv_object */
        //"add.l  r0, #4\n"
        //"mvtc   r0, usp\n"
        "rte\n"
        ".align 2\n"
        "pendsv_object_ptr: .word _pendsv_object\n"
        "nlr_jump_ptr: .word _nlr_jump\n"
    );
#endif
}

