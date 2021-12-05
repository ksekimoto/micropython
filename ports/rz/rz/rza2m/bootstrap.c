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

#include <stdio.h>
#include <stdint.h>
#include "iodefine.h"
#include "common.h"
#include "bootstrap.h"

static inline void rx_ethernet_enable(uint32_t ch) {
    volatile uint8_t dummy;
    if (ch == 0) {
        CPG.STBCR6.BIT.MSTP65 = 0;
    } else {
        CPG.STBCR6.BIT.MSTP64 = 0;
    }
    CPG.STBCR6.BIT.MSTP62 = 0;
    dummy = CPG.STBCR6.BYTE;
    (void)dummy;    // to suppress gcc warning
}

static inline void rx_ethernet_disable(uint32_t ch) {
    volatile uint8_t dummy;
    if (ch == 0) {
        CPG.STBCR6.BIT.MSTP65 = 1;
        dummy = CPG.STBCR6.BYTE;
    } else {
        CPG.STBCR6.BIT.MSTP64 = 1;
        dummy = CPG.STBCR6.BYTE;
    }
    CPG.STBCR6.BIT.MSTP62 = 1;
    dummy = CPG.STBCR6.BYTE;
    (void)dummy;    // to suppress gcc warning
}

static inline void rx_ethernet_MII_mode(uint32_t ch) {
    if (ch == 0) {
        /* ==== MII Pins setting ==== */
        /* Clear PDR and PMR */
        #if 0
        PORTE.PDR.BIT.PDR5 = 0;
        PORTE.PDR.BIT.PDR6 = 0;
        PORTG.PDR.BIT.PDR0 = 0;
        PORTG.PDR.BIT.PDR4 = 0;
        PORT6.PDR.BIT.PDR1 = 0;
        PORT6.PDR.BIT.PDR2 = 0;
        PORT6.PDR.BIT.PDR3 = 0;
        PORTG.PDR.BIT.PDR1 = 0;
        PORTG.PDR.BIT.PDR2 = 0;
        PORTE.PDR.BIT.PDR0 = 0;
        PORTE.PDR.BIT.PDR3 = 0;
        PORTG.PDR.BIT.PDR5 = 0;
        PORTE.PDR.BIT.PDR1 = 0;
        PORTE.PDR.BIT.PDR2 = 0;
        PORTG.PDR.BIT.PDR6 = 0;
        PORTG.PDR.BIT.PDR7 = 0;
        PORTE.PDR.BIT.PDR4 = 0;
        PORTG.PDR.BIT.PDR3 = 0;
        PORT6.PDR.BIT.PDR0 = 0;

        PORTE.PMR.BIT.PMR5 = 0;
        PORTE.PMR.BIT.PMR6 = 0;
        PORTG.PMR.BIT.PMR0 = 0;
        PORTG.PMR.BIT.PMR4 = 0;
        PORT6.PMR.BIT.PMR1 = 0;
        PORT6.PMR.BIT.PMR2 = 0;
        PORT6.PMR.BIT.PMR3 = 0;
        PORTG.PMR.BIT.PMR1 = 0;
        PORTG.PMR.BIT.PMR2 = 0;
        PORTE.PMR.BIT.PMR0 = 0;
        PORTE.PMR.BIT.PMR3 = 0;
        PORTG.PMR.BIT.PMR5 = 0;
        PORTE.PMR.BIT.PMR1 = 0;
        PORTE.PMR.BIT.PMR2 = 0;
        PORTG.PMR.BIT.PMR6 = 0;
        PORTG.PMR.BIT.PMR7 = 0;
        PORTE.PMR.BIT.PMR4 = 0;
        PORTG.PMR.BIT.PMR3 = 0;
        PORT6.PMR.BIT.PMR0 = 0;
        #endif
        GPIO.PWPR.BIT.B0WI = 0;
        GPIO.PWPR.BIT.PFSWE = 1;
        /* Select ethernet mode */
        GPIO.PFENET.BIT.PHYMODE0 = 1;   /* MII mode */
        GPIO.PE5PFS.BYTE = 0x01;
        GPIO.PE6PFS.BYTE = 0x01;
        GPIO.PG0PFS.BYTE = 0x01;
        GPIO.PG4PFS.BYTE = 0x01;
        GPIO.P61PFS.BYTE = 0x01;
        GPIO.P62PFS.BYTE = 0x01;
        GPIO.P63PFS.BYTE = 0x01;
        GPIO.PG1PFS.BYTE = 0x01;
        GPIO.PG2PFS.BYTE = 0x01;
        GPIO.PE0PFS.BYTE = 0x01;
        GPIO.PE3PFS.BYTE = 0x01;
        GPIO.PG5PFS.BYTE = 0x01;
        GPIO.PE1PFS.BYTE = 0x01;
        GPIO.PE2PFS.BYTE = 0x01;
        GPIO.PG6PFS.BYTE = 0x01;
        GPIO.PG7PFS.BYTE = 0x01;
        GPIO.PE4PFS.BYTE = 0x01;
        GPIO.PG3PFS.BYTE = 0x01;
        GPIO.PWPR.BIT.PFSWE = 0;
        GPIO.PWPR.BIT.B0WI = 1;

        PORTE.PMR.BIT.PMR5 = 1;
        PORTE.PMR.BIT.PMR6 = 1;
        PORTG.PMR.BIT.PMR0 = 1;
        PORTG.PMR.BIT.PMR4 = 1;
        PORT6.PMR.BIT.PMR1 = 1;
        PORT6.PMR.BIT.PMR2 = 1;
        PORT6.PMR.BIT.PMR3 = 1;
        PORTG.PMR.BIT.PMR1 = 1;
        PORTG.PMR.BIT.PMR2 = 1;
        PORTE.PMR.BIT.PMR0 = 1;
        PORTE.PMR.BIT.PMR3 = 1;
        PORTG.PMR.BIT.PMR5 = 1;
        PORTE.PMR.BIT.PMR1 = 1;
        PORTE.PMR.BIT.PMR2 = 1;
        PORTG.PMR.BIT.PMR6 = 1;
        PORTG.PMR.BIT.PMR7 = 1;
        PORTE.PMR.BIT.PMR4 = 1;
        PORTG.PMR.BIT.PMR3 = 1;
    } else {
        GPIO.PWPR.BIT.B0WI = 0;
        GPIO.PWPR.BIT.PFSWE = 1;
        /* Select ethernet mode */
        GPIO.PFENET.BIT.PHYMODE1 = 1;   /* MII mode */
        GPIO.P33PFS.BYTE = 0x01;
        GPIO.P34PFS.BYTE = 0x01;
        GPIO.PC0PFS.BYTE = 0x03;
        GPIO.PC4PFS.BYTE = 0x03;
        GPIO.PK0PFS.BYTE = 0x01;
        GPIO.PK1PFS.BYTE = 0x01;
        GPIO.PK2PFS.BYTE = 0x01;
        GPIO.PC1PFS.BYTE = 0x03;
        GPIO.PC2PFS.BYTE = 0x03;
        GPIO.PK3PFS.BYTE = 0x01;
        GPIO.P31PFS.BYTE = 0x01;
        GPIO.PC5PFS.BYTE = 0x03;
        GPIO.PK4PFS.BYTE = 0x01;
        GPIO.P35PFS.BYTE = 0x01;
        GPIO.PC6PFS.BYTE = 0x03;
        GPIO.PC7PFS.BYTE = 0x03;
        GPIO.P32PFS.BYTE = 0x01;
        GPIO.PC3PFS.BYTE = 0x03;
        GPIO.PWPR.BIT.PFSWE = 0;
        GPIO.PWPR.BIT.B0WI = 1;

        PORT3.PMR.BIT.PMR3 = 1;
        PORT3.PMR.BIT.PMR4 = 1;
        PORTC.PMR.BIT.PMR0 = 1;
        PORTC.PMR.BIT.PMR4 = 1;
        PORTK.PMR.BIT.PMR0 = 1;
        PORTK.PMR.BIT.PMR1 = 1;
        PORTK.PMR.BIT.PMR2 = 1;
        PORTC.PMR.BIT.PMR1 = 1;
        PORTC.PMR.BIT.PMR2 = 1;
        PORTK.PMR.BIT.PMR3 = 1;
        PORT3.PMR.BIT.PMR1 = 1;
        PORTC.PMR.BIT.PMR5 = 1;
        PORTK.PMR.BIT.PMR4 = 1;
        PORT3.PMR.BIT.PMR5 = 1;
        PORTC.PMR.BIT.PMR6 = 1;
        PORTC.PMR.BIT.PMR7 = 1;
        PORT3.PMR.BIT.PMR2 = 1;
        PORTC.PMR.BIT.PMR3 = 1;
    }
}

// static void clock_init(void) {
// }

void bootstrap(void) {
    // clock_init();
    rx_ethernet_enable(ETH_CH);
    rx_ethernet_MII_mode(ETH_CH);
}
