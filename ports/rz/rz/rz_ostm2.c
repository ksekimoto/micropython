/*
 * Copyright (c) 2020, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
//#define OSTM2_EXE_PRI   15  /* lowest */
#define OSTM2_INT_CNT

static bool rz_ostm2_irq_active = false;
static unsigned long rz_ostm2_irq_period = (unsigned long)OSTM2_DEF_PER;
static void *rz_ostm2_irq_handler_func = (void *)0;

void rz_ostm2_set_irq_period (uint32_t ms) {
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
            //uint32_t irq_state = raise_irq_pri(OSTM2_EXE_PRI);
            (*(void (*)())rz_ostm2_irq_handler_func)();
            //restore_irq_pri(irq_state);
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
    OSTM2.OSTMnTS.BYTE  = 0x1;  /* Start the counter */
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
