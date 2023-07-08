/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Kentaro Sekimoto
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
#include <stdint.h>
#include "iodefine.h"
#include "bootstrap.h"

static inline void rx_ethernet_enable(void) {
    SYSTEM.PRCR.WORD = 0xA502;          /* protect off */
    SYSTEM.MSTPCRB.BIT.MSTPB15 = 0;     /* EtherC, EDMAC */
    // BSC.BEREN.BIT.TOEN = 1;
    SYSTEM.PRCR.WORD = 0xA500;          /* protect on */
}

static inline void rx_ethernet_disable(void) {
    SYSTEM.PRCR.WORD = 0xA502;          /* protect off */
    SYSTEM.MSTPCRB.BIT.MSTPB15 = 1;     /* EtherC, EDMAC */
    SYSTEM.PRCR.WORD = 0xA500;          /* protect on */
}

static inline void rx_ethernet_RMII_mode(void) {
    /* ==== RMII Pins setting ==== */
    /*
    Pin Functions : Port
    --------------------
    ET0_MDIO       : PA3
    ET0_MDC        : PA4
    ET0_LINKSTA    : PA5
    RMII0_RXD1     : PB0    P74
    RMII0_RXD0     : PB1    P75
    REF50CK0       : PB2    P76
    RMII0_RX_ER    : PB3    P77
    RMII0_TXD_EN   : PB4    P80
    RMII0_TXD0     : PB5    P81
    RMII0_TXD1     : PB6    P82
    RMII0_CRS_DV   : PB7    P83
    */
    /* Clear PDR and PMR */
    PORTA.PDR.BIT.B3 = 0;
    PORTA.PDR.BIT.B4 = 0;
    PORTA.PDR.BIT.B5 = 0;
    PORTB.PDR.BIT.B0 = 0;
    PORTB.PDR.BIT.B1 = 0;
    PORTB.PDR.BIT.B2 = 0;
    PORTB.PDR.BIT.B3 = 0;
    PORTB.PDR.BIT.B4 = 0;
    PORTB.PDR.BIT.B5 = 0;
    PORTB.PDR.BIT.B6 = 0;
    PORTB.PDR.BIT.B7 = 0;

    PORTA.PMR.BIT.B3 = 0;
    PORTA.PMR.BIT.B4 = 0;
    PORTA.PMR.BIT.B5 = 0;
    PORTB.PMR.BIT.B0 = 0;
    PORTB.PMR.BIT.B1 = 0;
    PORTB.PMR.BIT.B2 = 0;
    PORTB.PMR.BIT.B3 = 0;
    PORTB.PMR.BIT.B4 = 0;
    PORTB.PMR.BIT.B5 = 0;
    PORTB.PMR.BIT.B6 = 0;
    PORTB.PMR.BIT.B7 = 0;
    /* Write protect off */
    MPC.PWPR.BYTE = 0x00;       /* PWPR.PFSWE write protect off */
    MPC.PWPR.BYTE = 0x40;       /* PFS register write protect off */
    MPC.PA3PFS.BYTE = 0x11;
    MPC.PA4PFS.BYTE = 0x11;
    MPC.PA5PFS.BYTE = 0x11;
    MPC.PB0PFS.BYTE = 0x12;
    MPC.PB1PFS.BYTE = 0x12;
    MPC.PB2PFS.BYTE = 0x12;
    MPC.PB3PFS.BYTE = 0x12;
    MPC.PB4PFS.BYTE = 0x12;
    MPC.PB5PFS.BYTE = 0x12;
    MPC.PB6PFS.BYTE = 0x12;
    MPC.PB7PFS.BYTE = 0x12;
    /* Write protect on */
    MPC.PWPR.BYTE = 0x80;       /* PFS register write protect on */
    /* Select ethernet mode */
    MPC.PFENET.BIT.PHYMODE0 = 0; /* RMII mode */
    /* Switch to the selected input/output function */
    PORTA.PMR.BIT.B3 = 1;
    PORTA.PMR.BIT.B4 = 1;
    PORTA.PMR.BIT.B5 = 1;
    PORTB.PMR.BIT.B0 = 1;
    PORTB.PMR.BIT.B1 = 1;
    PORTB.PMR.BIT.B2 = 1;
    PORTB.PMR.BIT.B3 = 1;
    PORTB.PMR.BIT.B4 = 1;
    PORTB.PMR.BIT.B5 = 1;
    PORTB.PMR.BIT.B6 = 1;
    PORTB.PMR.BIT.B7 = 1;
}

void HardwareSetup(void) {
    clock_init();
    rx_ethernet_enable();
    rx_ethernet_RMII_mode();
}
