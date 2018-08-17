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
#define CMT_VAL 0x0040;     // CMIE is Enable,CKS is PCLK/8
#elif CLKDEV == 32
#define CMT_VAL 0x0041;     // CMIE is Enable,CKS is PCLK/32
#elif CLKDEV == 128
#define CMT_VAL 0x0042;     // CMIE is Enable,CKS is PCLK/128
#else
#define CMT_VAL 0x0043;     // CMIE is Enable,CKS is PCLK/512
#endif

#define DELAY_CH    0

volatile struct st_cmt0 *CMTN[4] = {
    (volatile struct st_cmt0 *)0x88002,
    (volatile struct st_cmt0 *)0x88008,
    (volatile struct st_cmt0 *)0x88012,
    (volatile struct st_cmt0 *)0x88018
};

volatile static unsigned long cmcnt[4] = { 0L, 0L, 0L, 0L };

static void isr_timer(unsigned int ch)
{
    cmcnt[ch] += 1L;
}

void __attribute__ ((interrupt)) INT_Excep_CMT0_CMI0(void)
{
    isr_timer(0);
}
void __attribute__ ((interrupt)) INT_Excep_CMT1_CMI1(void)
{
    isr_timer(1);
}
void __attribute__ ((interrupt)) INT_Excep_CMT2_CMI2(void)
{
    isr_timer(2);
}
void __attribute__ ((interrupt)) INT_Excep_CMT3_CMI3(void)
{
    isr_timer(3);
}

void timer_init(unsigned int ch)
{
    int i;
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    for (i = 0; i < 4; i++)
        cmcnt[i] = 0L;
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
    switch(ch) {
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

void timer_set_count(unsigned int ch, unsigned int count)
{
    volatile struct st_cmt0 *cmtn = CMTN[ch];
    cmtn->CMCNT = (unsigned short)0;
    do {
        cmtn->CMCOR = (unsigned short)count;
    } while (cmtn->CMCOR != (unsigned short)count);
}

void udelay_init(void)
{
    timer_init(DELAY_CH);
    timer_set_count(DELAY_CH, 60);
}

void udelay(int m)
{
    volatile unsigned long start;
    if (m >= 10) {
        m /= 10;
        start = cmcnt[DELAY_CH];
        while (cmcnt[DELAY_CH] - start < (unsigned long)m) {
            __asm__ __volatile__ ("nop");
        }
    } else {
        while (m-- > 0) {
            __asm__ __volatile__ ("nop");
        }
    }
}

unsigned long utick(void)
{
    return cmcnt[DELAY_CH]*10;
}

unsigned long mtick(void)
{
    return cmcnt[DELAY_CH]/100;
}
