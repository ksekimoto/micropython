/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2020 Kentaro Sekimoto
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
#include "shared/runtime/interrupt_char.h"
#include "pendsv.h"
#include "irq.h"
#include "rz_ostm2.h"

// This variable is used to save the exception object between a ctrl-C and the
// PENDSV call that actually raises the exception.  It must be non-static
// otherwise gcc-5 optimises it away.  It can point to the heap but is not
// traced by GC.  This is okay because we only ever set it to
// mp_kbd_exception which is in the root-pointer set.
void *pendsv_object;

#if defined(PENDSV_DISPATCH_NUM_SLOTS)
uint32_t pendsv_dispatch_active;
pendsv_dispatch_t pendsv_dispatch_table[PENDSV_DISPATCH_NUM_SLOTS];
void pendsv_dispatch_handler(void);
#endif

void pendsv_init(void) {
    #if defined(PENDSV_DISPATCH_NUM_SLOTS)
    pendsv_dispatch_active = false;
    #endif
    // set PendSV interrupt at lowest priority
    rz_ostm2_set_irq_period(1);
    rz_ostm2_set_irq_handler((void *)pendsv_dispatch_handler);
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
    if (MP_STATE_MAIN_THREAD(mp_pending_exception) == MP_OBJ_NULL) {
        mp_sched_keyboard_interrupt();
    } else {
        MP_STATE_MAIN_THREAD(mp_pending_exception) = MP_OBJ_NULL;
        pendsv_object = &MP_STATE_VM(mp_kbd_exception);
    }
}

#if defined(PENDSV_DISPATCH_NUM_SLOTS)
void pendsv_schedule_dispatch(size_t slot, pendsv_dispatch_t f) {
    pendsv_dispatch_table[slot] = f;
    pendsv_dispatch_active = true;
}

void pendsv_dispatch_handler(void) {
    for (size_t i = 0; i < PENDSV_DISPATCH_NUM_SLOTS; ++i) {
        if (pendsv_dispatch_table[i] != NULL) {
            pendsv_dispatch_t f = pendsv_dispatch_table[i];
            pendsv_dispatch_table[i] = NULL;
            f();
        }
    }
}
#endif

// Replace with rz_ostm2 timer with lowest priority
// __attribute__((naked)) void PendSV_Handler(void) {}
