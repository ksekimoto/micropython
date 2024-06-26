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

#include <stdio.h>
#include "iodefine.h"
#include "common.h"
#include "rx_timer.h"

#if defined(RX63N)
#define TIM_1MS_COUNT   6000
#define TIM_1US_COUNT   60
#elif defined(RX65N)
#define TIM_1MS_COUNT   7500
#define TIM_1US_COUNT   75
#else
#error "RX MCU Series is not specified."
#endif

#define USEC_CH    0
#define MSEC_CH    1

volatile struct st_cmt0 *CMTN[4] = {
    (volatile struct st_cmt0 *)0x88002,
    (volatile struct st_cmt0 *)0x88008,
    (volatile struct st_cmt0 *)0x88012,
    (volatile struct st_cmt0 *)0x88018
};

volatile static unsigned long cmt_count[4] = { 0L, 0L, 0L, 0L };
volatile static unsigned long cmt_compare[4] = { 0L, 0L, 0L, 0L };
volatile static unsigned long cmt_period[4] = { 0L, 0L, 0L, 0L };

static CMT_TIMER_FUNC cmt_timer_func[4] = {0};
static void *cmt_timer_param[4] = {0};

static void isr_timer(unsigned int ch) {
    cmt_count[ch] += 1L;
}

void SysTick_Handler(void);

/*
 * 10usec timer
 */
void __attribute__ ((interrupt)) INT_Excep_CMT0_CMI0(void) {
    isr_timer(0);
    cmt_timer_callback(0);
    if ((cmt_count[0] % 100UL) == 0) {
        SysTick_Handler();
    }
}

/*
 * 1msec timer
 */
void __attribute__ ((interrupt)) INT_Excep_CMT1_CMI1(void) {
    isr_timer(1);
    cmt_timer_callback(1);
}

void __attribute__ ((interrupt)) INT_Excep_CMT2_CMI2(void) {
    isr_timer(2);
    cmt_timer_callback(2);
}

void __attribute__ ((interrupt)) INT_Excep_CMT3_CMI3(void) {
    isr_timer(3);
    cmt_timer_callback(1);
}

void cmt_timer_set_callback(unsigned int ch, CMT_TIMER_FUNC func, void *param) {
    cmt_timer_func[ch] = func;
    cmt_timer_param[ch] = param;
}

void cmt_timer_callback(unsigned int ch) {
    if (cmt_timer_func[ch]) {
        (*cmt_timer_func[ch])(cmt_timer_param[ch]);
    }
}

unsigned int cmt_timer_get_prescale(unsigned int ch) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    unsigned int prescale = 0;
    switch (cmtn->CMCR.WORD) {
        case 0x40:
            prescale = 8;
            break;
        case 0x41:
            prescale = 32;
            break;
        case 0x42:
            prescale = 128;
            break;
        case 0x43:
            prescale = 512;
            break;
    }
    return prescale;
}

void cmt_timer_set_prescale(unsigned int ch, unsigned int prescale) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    unsigned short val = 0;
    switch (prescale) {
        case 8:
            val = 0x40;
            break;
        case 32:
            val = 0x41;
            break;
        case 128:
            val = 0x42;
            break;
        case 512:
            val = 0x43;
            break;
    }
    cmtn->CMCR.WORD = (unsigned short)val;
}

void cmt_timer_init(unsigned int ch, unsigned int prescale) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    cmt_count[ch] = 0L;
    SYSTEM.PRCR.WORD = 0xA502;
    if ((ch == 0) || (ch == 1)) {
        SYSTEM.MSTPCRA.BIT.MSTPA15 = 0;
    } else {
        SYSTEM.MSTPCRA.BIT.MSTPA14 = 0;
    }
    SYSTEM.PRCR.WORD = 0xA500;
    CMT.CMSTR0.WORD &= (ch == 0 ? 2 : 1);   // disable clock
    cmtn->CMCNT = 0;
    cmtn->CMCOR = 0xffff;
    cmt_timer_set_prescale(ch, prescale);

    switch (ch) {
        case 0:
            ICU.IPR[0x04].BIT.IPR = RX_PRI_TIM0; // IPR = 15 (15: highest priority)
            ICU.IER[0x03].BIT.IEN4 = 1;         // IER enable
            break;
        case 1:
            ICU.IPR[0x05].BIT.IPR = RX_PRI_TIM1; // IPR = 5 (15: highest priority)
            ICU.IER[0x03].BIT.IEN5 = 1;         // IER enable
            break;
        case 2:
            ICU.IPR[0x06].BIT.IPR = RX_PRI_TIM2; // IPR = 12 (15: highest priority)
            ICU.IER[0x03].BIT.IEN6 = 1;         // IER enable
            break;
        case 3:
            ICU.IPR[0x07].BIT.IPR = RX_PRI_TIM3; // IPR = 12 (15: highest priority)
            ICU.IER[0x03].BIT.IEN7 = 1;         // IER enable
            break;
        default:
            break;
    }
    CMT.CMSTR0.WORD |= (ch == 0 ? 1 : 2);   // enable clock
}

void cmt_timer_deinit(unsigned int ch) {
    cmt_count[ch] = 0L;
    switch (ch) {
        case 0:
            ICU.IER[0x03].BIT.IEN4 = 0;     // IER disable
            break;
        case 1:
            ICU.IER[0x03].BIT.IEN5 = 0;     // IER disable
            break;
        case 2:
            ICU.IER[0x03].BIT.IEN6 = 0;     // IER disable
            break;
        case 3:
            ICU.IER[0x03].BIT.IEN7 = 0;     // IER disable
            break;
        default:
            break;
    }
    cmt_timer_func[ch] = (CMT_TIMER_FUNC)0;
    cmt_timer_param[ch] = (void *)0;
}

void cmt_timer_disable_clk(unsigned int ch) {
    CMT.CMSTR0.WORD &= (ch == 0 ? 2 : 1);   // disable clock
}

void cmt_timer_eable_clk(unsigned int ch) {
    CMT.CMSTR0.WORD |= (ch == 0 ? 1 : 2);   // enable clock
}

unsigned int cmt_timer_get_counter(unsigned int ch) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    return (unsigned int)cmtn->CMCOR;
}

void cmt_timer_set_counter(unsigned int ch, unsigned int count) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    cmtn->CMCNT = (unsigned short)0;
    do {
        cmtn->CMCOR = (unsigned short)count;
    } while (cmtn->CMCOR != (unsigned short)count);
}

unsigned long cmt_timer_get_cnt(unsigned int ch) {
    return cmt_count[ch];
}

void cmt_timer_set_cnt(unsigned int ch, unsigned long cnt) {
    cmt_compare[ch] = cnt;
}

unsigned long cmt_timer_get_period(unsigned int ch) {
    return cmt_period[ch];
}

void cmt_timer_set_period(unsigned int ch, unsigned long period) {
    cmt_period[ch] = period;
}

void udelay_init(void) {
    cmt_timer_init(USEC_CH, 8);
    cmt_timer_set_counter(USEC_CH, TIM_1US_COUNT);
    cmt_timer_init(MSEC_CH, 8);
    cmt_timer_set_counter(MSEC_CH, TIM_1MS_COUNT);
}

void udelay(int m) {
    volatile unsigned long start;
    if (m >= 10) {
        m /= 10;
        start = cmt_count[USEC_CH];
        while (cmt_count[USEC_CH] - start < (unsigned long)m) {
            __asm__ __volatile__ ("nop");
        }
    } else {
        while (m-- > 0) {
            __asm__ __volatile__ ("nop");
        }
    }
}

unsigned long utick(void) {
    return cmt_count[USEC_CH] * 10UL;
}

void mdelay(int m) {
    volatile unsigned long start = cmt_count[MSEC_CH];
    while (cmt_count[MSEC_CH] - start < (unsigned long)m) {
        __asm__ __volatile__ ("nop");
    }
}

unsigned long mtick(void) {
    // return cmt_count[MSEC_CH];
    return cmt_count[USEC_CH] / 100UL;
}
