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

static void usb_clock_init(void)
{
    volatile int i;
    SYSTEM.PRCR.WORD = 0xA503;                 // Protect register (protect off)
    if (SYSTEM.RSTSR0.BIT.DPSRSTF == 1) { // Reset status register (Deep software standby reset check)
        if (SYSTEM.DPSIFR2.BIT.DUSBIF == 1) { // Deep standby interrupt flag register2
            SYSTEM.DPSIFR2.BIT.DUSBIF = 0;      // Clear USB Request
        }
        PORT3.PMR.BIT.B6 = 1;               // Port mode register
        PORT3.PMR.BIT.B7 = 1;               // Port mode register
        SYSTEM.MOSCCR.BYTE = 0x00;          // Main clock oscillator is operated
        SYSTEM.MOSCWTCR.BYTE = 0x0D;        // 131072 state
        SYSTEM.SOSCCR.BYTE = 0x01;          // Sub clock Oscillator is stopped
        SYSTEM.PLLCR.WORD = 0x0F00;     // PLIDIV = 12MHz(/1), STC = 192MHz(*16)
        SYSTEM.PLLCR2.BYTE = 0x00;          // PLL enable
        SYSTEM.PLLWTCR.BYTE = 0x0F;         // 4194304cycle(Default)
        for (i = 0; i < 600; i++) {
        }
        SYSTEM.SCKCR.LONG = 0x21032222; // ICK(96MHz)=PLL/2,BCK(24MHz)=PLL/8,FCK,PCK(48MHz)=PLL/4
        SYSTEM.SCKCR3.WORD = 0x0400;        // PLL
        SYSTEM.SCKCR2.BIT.UCK = 3;          // USB clock : 48MHz
        SYSTEM.MSTPCRA.LONG = 0x7FFFFFFF; // Module stop control register (Disable ACSE)
        SYSTEM.MSTPCRB.BIT.MSTPB19 = 0u;    // Enable USB0 module
        SYSTEM.SYSCR0.WORD = 0x5A03;        // External Bus Enable
        PORT5.PMR.BIT.B3 = 1;
        SYSTEM.BCKCR.BIT.BCLKDIV = 1;       // BCLK * 1/2
        SYSTEM.SCKCR.BIT.PSTOP1 = 0;        // BCLK Pin output enable
        SYSTEM.DPSBYCR.BIT.IOKEEP = 0;      // IO status keep disable
        SYSTEM.RSTSR0.BIT.DPSRSTF = 0;      // Deep software standby reset
    } else {
        PORT3.PMR.BIT.B6 = 1;
        PORT3.PMR.BIT.B7 = 1;
        SYSTEM.MOSCWTCR.BYTE = 0x0D;        // 131072 state
        SYSTEM.PLLCR.WORD = 0x0F00;
        SYSTEM.MOSCCR.BYTE = 0x00;          // EXTAL ON
        SYSTEM.PLLCR2.BYTE = 0x00;          // PLL ON
        SYSTEM.PLLWTCR.BYTE = 0x0F;
        for (i = 0; i < 300; i++) {
        }
        SYSTEM.SCKCR.LONG = 0x21832222;
        SYSTEM.SCKCR3.WORD = 0x0400;
        SYSTEM.SCKCR2.BIT.UCK = 3;
        SYSTEM.MSTPCRA.LONG = 0x7FFFFFFF;
        SYSTEM.MSTPCRB.BIT.MSTPB19 = 0u;    //  Enable USB0 module
        SYSTEM.SYSCR0.WORD = 0x5A03;
        PORT5.PMR.BIT.B3 = 1;
        SYSTEM.BCKCR.BIT.BCLKDIV = 1;
        SYSTEM.SCKCR.BIT.PSTOP1 = 0;
    }
}

void bootstrap(void)
{
    //SYSTEM.SCKCR.LONG = 0x21032200;     /* clock init: ICK=PLL/2, BCLK=PLL/8, PCLK=PLL/4 */
    usb_clock_init();
    MPC.PWPR.BIT.B0WI = 0;
    MPC.PWPR.BIT.PFSWE = 1;

    MPC.PFCSE.BIT.CS3E = 1;
    MPC.PFCSS0.BIT.CS3S = 2;
    MPC.PFAOE1.BIT.A17E = 1;

    BSC.CS3CR.WORD = 0x0001 | (2 << 4);
    BSC.CS3MOD.WORD = 0x8001;
    BSC.CS3WCR1.LONG = 0x01010101;
    BSC.CS3WCR2.LONG = 0x11110111;
    BSC.CS3REC.WORD = 0x0000;
}

