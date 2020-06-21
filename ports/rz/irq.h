/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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
#ifndef MICROPY_INCLUDED_RZ_IRQ_H
#define MICROPY_INCLUDED_RZ_IRQ_H

#include "irq_ctrl.h"

// Use this macro together with NVIC_SetPriority to indicate that an IRQn is non-negative,
// which helps the compiler optimise the resulting inline function.
#define IRQn_NONNEG(pri) ((pri) & 0x7f)

// these states correspond to values from query_irq, enable_irq and disable_irq
#define IRQ_STATE_DISABLED (0x00000080)
#define IRQ_STATE_ENABLED  (0x00000000)

// Enable this to get a count for the number of times each irq handler is called,
// accessible via pyb.irq_stats().
#define IRQ_ENABLE_STATS (0)

#if IRQ_ENABLE_STATS
#define IRQ_STATS_MAX   (128)
extern uint32_t irq_stats[IRQ_STATS_MAX];
#define IRQ_ENTER(irq) ++irq_stats[irq]
#define IRQ_EXIT(irq)
#else
#define IRQ_ENTER(irq)
#define IRQ_EXIT(irq)
#endif

static inline mp_uint_t query_irq(void) {
    uint32_t state;
    __asm__ __volatile__ (  "MRS %0,APSR\n\t"
                            "AND %0,%0,#0x80" : "=r" (state) : : );
    return state;
}

// enable_irq and disable_irq are defined inline in mpconfigport.h

static inline uint32_t raise_irq_pri(uint32_t pri) {
    uint32_t cur_pri = IRQ_GetPriorityMask();
    IRQ_SetPriorityMask(pri);
    return cur_pri;
}

// "state" should be the value returned from raise_irq_pri
static inline void restore_irq_pri(uint32_t state) {
    IRQ_SetPriorityMask(state);
}

MP_DECLARE_CONST_FUN_OBJ_0(pyb_wfi_obj);
MP_DECLARE_CONST_FUN_OBJ_0(pyb_disable_irq_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_enable_irq_obj);
MP_DECLARE_CONST_FUN_OBJ_0(pyb_irq_stats_obj);

// IRQ priority definitions.
//
// Lower number implies higher interrupt priority.
//
// The default priority grouping is set to NVIC_PRIORITYGROUP_4 in the
// HAL_Init function. This corresponds to 4 bits for the priority field
// and 0 bits for the sub-priority field (which means that for all intensive
// purposes that the sub-priorities below are ignored).
//
// While a given interrupt is being processed, only higher priority (lower number)
// interrupts will preempt a given interrupt. If sub-priorities are active
// then the sub-priority determines the order that pending interrupts of
// a given priority are executed. This is only meaningful if 2 or more
// interrupts of the same priority are pending at the same time.
//
// The priority of the SysTick timer is determined from the TICK_INT_PRIORITY
// value which is normally set to 0 in the stm32f4xx_hal_conf.h file.
//
// The following interrupts are arranged from highest priority to lowest
// priority to make it a bit easier to figure out.

#define IRQ_PRI_SYSTICK         0

// The UARTs have no FIFOs, so if they don't get serviced quickly then characters
// get dropped. The handling for each character only consumes about 0.5 usec
#define IRQ_PRI_UART            1

// SDIO must be higher priority than DMA for SDIO DMA transfers to work.
#define IRQ_PRI_SDIO            4

// DMA should be higher priority than USB, since USB Mass Storage calls
// into the sdcard driver which waits for the DMA to complete.
#define IRQ_PRI_DMA             5

// Flash IRQ (used for flushing storage cache) must be at the same priority as
// the USB IRQs, so that the IRQ priority can be raised to this level to disable
// both the USB and cache flushing, when storage transfers are in progress.
#define IRQ_PRI_FLASH           6

#define IRQ_PRI_OTG_FS          6
#define IRQ_PRI_OTG_HS          6
#define IRQ_PRI_TIM5            6

#define IRQ_PRI_CAN             7

#define IRQ_PRI_SPI             8

// Interrupt priority for non-special timers.
#define IRQ_PRI_TIMX            13

#define IRQ_PRI_EXTINT          14

// PENDSV should be at the lowst priority so that other interrupts complete
// before exception is raised.
#define IRQ_PRI_PENDSV          15
#define IRQ_PRI_RTC_WKUP        15

#endif // MICROPY_INCLUDED_RZ_IRQ_H
