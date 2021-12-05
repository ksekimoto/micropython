/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
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
/*
 * This implementation is based on mbed us_ticker.c.
 */

#include <stdbool.h>
#include "RZ_A2M.h"
#include "iodefine.h"
#include "irq_ctrl.h"
#include "rz_ostm2.h"

static inline uint32_t raise_irq_pri(uint32_t pri) {
    uint32_t cur_pri = IRQ_GetPriorityMask();
    IRQ_SetPriorityMask(pri);
    return cur_pri;
}

// "state" should be the value returned from raise_irq_pri
static inline void restore_irq_pri(uint32_t state) {
    IRQ_SetPriorityMask(state);
}

#define OSTM2_1MS       ((CM0_RENESAS_RZ_A2_P0_CLK * 2) / 1000)
#define OSTM2_100US     ((CM0_RENESAS_RZ_A2_P0_CLK * 2) / 10000)
#define OSTM2_DEF_PER   OSTM2_1MS
#define OSTM2_INT_PRI   15  /* lowest */
// #define OSTM2_EXE_PRI   15  /* lowest */
#define OSTM2_INT_CNT

static bool rz_ostm2_irq_active = false;
static unsigned long rz_ostm2_irq_period = (unsigned long)OSTM2_DEF_PER;
static void *rz_ostm2_irq_handler_func = (void *)0;

void rz_ostm2_set_irq_period(uint32_t ms) {
    GIC_DisableIRQ(OSTMI2_IRQn);
    uint32_t cmp = 0;
    if (((uint64_t)ms * (uint64_t)OSTM2_1MS) > (uint64_t)0xffffffff) {
        cmp = 0xffffffff;
    } else if (ms == 0) {
        cmp = OSTM2_DEF_PER;
    } else {
        cmp = ms * OSTM2_DEF_PER;
    }
    rz_ostm2_irq_period = (unsigned long)cmp;
    OSTM2.OSTMnTT.BYTE = 0x1;   /* Stop the counter */
    OSTM2.OSTMnCMP.LONG = OSTM2.OSTMnCNT.LONG + rz_ostm2_irq_period;
    OSTM2.OSTMnTS.BYTE = 0x1;   /* Start the counter */
    GIC_EnableIRQ(OSTMI2_IRQn);
}

void rz_ostm2_set_irq_handler(void *func) {
    while (rz_ostm2_irq_active) {
        ;
    }
    GIC_DisableIRQ(OSTMI2_IRQn);
    rz_ostm2_irq_handler_func = func;
    GIC_EnableIRQ(OSTMI2_IRQn);
}

void rz_ostm2_irq_handler(void) {
    OSTM2.OSTMnTT.BYTE = 0x1;   /* Stop the counter */
    if (!rz_ostm2_irq_active) {
        rz_ostm2_irq_active = true;
        if (rz_ostm2_irq_handler_func) {
            // uint32_t irq_state = raise_irq_pri(OSTM2_EXE_PRI);
            (*(void (*)())rz_ostm2_irq_handler_func)();
            // restore_irq_pri(irq_state);
        }
        rz_ostm2_irq_active = false;
    }
    OSTM2.OSTMnCMP.LONG = OSTM2.OSTMnCNT.LONG + rz_ostm2_irq_period;
    OSTM2.OSTMnTS.BYTE = 0x1;   /* Start the counter */
}

void rz_ostm2_init(void) {
    GIC_DisableIRQ(OSTMI2_IRQn);
    GIC_ClearPendingIRQ(OSTMI2_IRQn);

    volatile uint8_t dummy_buf;
    CPG.STBCR3.BYTE &= ~(0x10u);
    dummy_buf = CPG.STBCR3.BYTE;
    (void)dummy_buf;

    // timer settings
    OSTM2.OSTMnTT.BYTE = 0x01;  /* Stop the counter */
    OSTM2.OSTMnCTL.BYTE = 0x02; /* Free running timer mode, interrupt enable */
    OSTM2.OSTMnCMP.LONG = OSTM2.OSTMnCNT.LONG + rz_ostm2_irq_period;
    OSTM2.OSTMnTS.BYTE = 0x1;   /* Start the counter */
    rz_ostm2_irq_active = false;

    InterruptHandlerRegister(OSTMI2_IRQn, (void (*)())rz_ostm2_irq_handler);
    GIC_SetPriority(OSTMI2_IRQn, OSTM2_INT_PRI);
    GIC_SetConfiguration(OSTMI2_IRQn, 3);
}

void rz_ostm2_deinit(void) {
    GIC_DisableIRQ(OSTMI2_IRQn);
    GIC_ClearPendingIRQ(OSTMI2_IRQn);

    volatile uint8_t dummy_buf;
    CPG.STBCR3.BYTE |= 0x10u;
    dummy_buf = CPG.STBCR3.BYTE;
    (void)dummy_buf;
    rz_ostm2_irq_active = false;
}
