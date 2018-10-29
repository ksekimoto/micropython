/*
 * Copyright (c) 2018, Kentaro Sekimoto
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

#include "iodefine.h"
#include "rx63n_timer.h"

#define CLKDEV  8
// PCLK = 48000000Hz default
#if CLKDEV == 8
#define CMT_VAL 0x0040;     // CMIE is Enable,CKS is PCLK/8     clk=1/6(us)
#elif CLKDEV == 32
#define CMT_VAL 0x0041;     // CMIE is Enable,CKS is PCLK/32    clk=1/1.5(us)
#elif CLKDEV == 128
#define CMT_VAL 0x0042;     // CMIE is Enable,CKS is PCLK/128   clk=32/12(us)
#else
#define CMT_VAL 0x0043;     // CMIE is Enable,CKS is PCLK/512   clk=128/3(us)
#endif

/*
 * Prescale: 8
 * freq = 1000(1/s) = 1000(us) -> 6000
 * freq = 10000(1/s) = 100(us) -> 600
 * freq = 100000(1/s) = 10(us) -> 60
 * Prescale: 32
 * freq = 100(1/s) = 10000(us) -> 15000
 * freq = 1000(1/s) = 1000(us) -> 1500
 * freq = 10000(1/s) = 100(us) -> 150
 * Prescale: 128
 * freq = 10(1/s) = 100000(us) -> 37500
 * freq = 100(1/s) = 10000(us) -> 3750
 * freq = 1000(1/s) = 1000(us) -> 375
 * Prescale: 512
 * freq = 1(1/s) = 1000000(us) -> 23437.5 -> 23437
 * freq = 10(1/s) = 100000(us) -> 2343.75 -> 2344
 * freq = 100(1/s) = 10000(us) -> 234.375 -> 234
 */

#define DELAY_CH    0

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

void __attribute__ ((interrupt)) INT_Excep_CMT0_CMI0(void) {
    isr_timer(0);
    cmt_timer_callback(0);
}
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

void cmt_timer_init(unsigned int ch) {
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
    cmtn->CMCR.WORD = CMT_VAL
    ;
    switch (ch) {
    case 0:
        ICU.IPR[0x04].BIT.IPR = 0xf;        // IPR = 14 (15: highest priority)
        ICU.IER[0x03].BIT.IEN4 = 1;         // IER enable
        break;
    case 1:
        ICU.IPR[0x05].BIT.IPR = 0xe;        // IPR = 14 (15: highest priority)
        ICU.IER[0x03].BIT.IEN5 = 1;         // IER enable
        break;
    case 2:
        ICU.IPR[0x06].BIT.IPR = 0xe;        // IPR = 14 (15: highest priority)
        ICU.IER[0x03].BIT.IEN6 = 1;         // IER enable
        break;
    case 3:
        ICU.IPR[0x07].BIT.IPR = 0xe;        // IPR = 14 (15: highest priority)
        ICU.IER[0x03].BIT.IEN7 = 1;         // IER enable
    default:
        break;
    }
    CMT.CMSTR0.WORD |= (ch == 0 ? 1 : 2);   // enable clock
}

void cmt_timer_deinit(unsigned int ch) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    cmt_count[ch] = 0L;
    switch (ch) {
    case 0:
        ICU.IER[0x03].BIT.IEN4 = 0;         // IER disable
        break;
    case 1:
        ICU.IER[0x03].BIT.IEN5 = 0;         // IER disable
        break;
    case 2:
        ICU.IER[0x03].BIT.IEN6 = 0;         // IER disable
        break;
    case 3:
        ICU.IER[0x03].BIT.IEN7 = 0;         // IER disable
    default:
        break;
    }
    cmt_timer_func[ch] = (CMT_TIMER_FUNC)0;
    cmt_timer_param[ch] = (void *)0;
}

void cmt_timer_disable_clk(unsigned int ch) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    CMT.CMSTR0.WORD &= (ch == 0 ? 2 : 1);   // disable clock
}

void cmt_timer_eable_clk(unsigned int ch) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    CMT.CMSTR0.WORD |= (ch == 0 ? 1 : 2);   // enable clock
}

unsigned int cmt_timer_get_prescale(unsigned int ch) {
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    unsigned int prescale;
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
    unsigned short val;
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
    cmt_timer_init(DELAY_CH);
    cmt_timer_set_counter(DELAY_CH, 60);
}

void udelay(int m) {
    volatile unsigned long start;
    if (m >= 10) {
        m /= 10;
        start = cmt_count[DELAY_CH];
        while (cmt_count[DELAY_CH] - start < (unsigned long)m) {
            __asm__ __volatile__ ("nop");
        }
    } else {
        while (m-- > 0) {
            __asm__ __volatile__ ("nop");
        }
    }
}

unsigned long utick(void) {
    return cmt_count[DELAY_CH] * 10;
}

unsigned long mtick(void) {
    return cmt_count[DELAY_CH] / 100;
}
