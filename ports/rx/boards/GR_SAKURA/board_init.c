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

static void clock_init(void) {
    volatile uint32_t t;
    /* ---- Disable write protection ---- */
    SYSTEM.PRCR.WORD = 0xA503;              /* Enable writing to registers */
                                            /* related to the clock generation circuit. */
                                            /* Enable writing to registers */
                                            /* related to the low power consumption function. */
    /* ---- Turn off the HOCO power supply ---- */
    SYSTEM.HOCOPCR.BYTE = 0x01;             /* HOCO power supply is turned off. */
    /* ---- Stop the sub-clock ---- */
    SYSTEM.SOSCCR.BYTE = 0x01;              /* Sub-clock oscillator is stopped. */
    while (SYSTEM.SOSCCR.BYTE != 0x01) {     /* Confirm that the written value can be read correctly. */
    }
    RTC.RCR3.BYTE = 0x0C;                   /* Sub-clock oscillator is stopped. */
    while (RTC.RCR3.BYTE != 0x0C) {          /* Confirm that the written value can be read correctly. */
    }
    /* ---- Set wait time until the main clock oscillator stabilizes ---- */
    SYSTEM.MOSCWTCR.BYTE = 0x0D;            /* Wait time is 131072 cycles (approx. 10.92 ms). */
    /* ---- Operate the main clock oscillator ---- */
    SYSTEM.MOSCCR.BYTE = 0x00;              /* Main clock oscillator is operating. */
    while (SYSTEM.MOSCCR.BYTE != 0x00) {     /* Confirm that the written value can be read correctly. */
    }
    /* ---- Set the main clock oscillator forced oscillation control ---- */
    SYSTEM.MOFCR.BYTE = 0x00;               /* Don't forcedly oscillated */
    while (SYSTEM.MOFCR.BYTE != 0x00) {      /* Confirm that the written value can be read correctly. */
    }
    /* ---- Set the PLL division ratio and multiplication factor ---- */
    SYSTEM.PLLCR.WORD = 0x0F00;             /* PLL input division ratio is no division. */
                                            /* Frequency multiplication factor is multiply-by-16. */
    /* ---- Set wait time until the PLL clock oscillator stabilizes ---- */
    SYSTEM.PLLWTCR.BYTE = 0x0E;             /* Wait time is 2097152 cycles (approx. 10.92 ms) */
    /* ---- Operate the PLL clock oscillator ---- */
    SYSTEM.PLLCR2.BYTE = 0x00;              /* PLL is operating. */
    /* ---- Wait processing for the clock oscillation stabilization ---- */
    for (t = 0; t < 20000; t++) {}
    /* ---- Set the internal clock division ratio ---- */
    /*  NOTE:To use ETHERC, the following conditions must be met.
        12.5 MHz <= PCLKA <= 100 MHz, PCLKA frequency = ICLK frequency  */
    SYSTEM.SCKCR.LONG = 0x21C21211;         /* FlashIF clock (FCLK), divide-by-4 */
                                            /* System clock (ICLK), divide-by-2 */
                                            /* BCLK pin output is disabled. (Fixed high) */
                                            /* SDCLK pin output is disabled. (Fixed high) */
                                            /* External bus clock (BCLK), divide-by-4 */
                                            /* Peripheral module clock A (PCLKA), divide-by-2 */
                                            /* Peripheral module clock B (PCLKB), divide-by-4 */
    while (SYSTEM.SCKCR.LONG != 0x21C21211) { /* Confirm that the written value can be read correctly. */
    }
    SYSTEM.SCKCR2.WORD = 0x0032;            /* USB is in use. */
                                            /* IEBus clock (IECLK), divide-by-64 */
    while (SYSTEM.SCKCR2.WORD != 0x0032) {   /* Confirm that the written value can be read correctly. */
    }
    /* ---- Set the BCLK pin output ---- */
    SYSTEM.BCKCR.BYTE = 0x01;               /* BCLK divided by 2 */
    /* ---- Set the internal clock source ---- */
    SYSTEM.SCKCR3.WORD = 0x0400;            /* PLL circuit is selected. */
    while (SYSTEM.SCKCR3.WORD != 0x0400) {   /* Confirm that the written value can be read correctly. */
    }
    SYSTEM.MSTPCRB.BIT.MSTPB15 = 0u;    //  Enable EDMAC module
    SYSTEM.MSTPCRB.BIT.MSTPB19 = 0u;    //  Enable USB0 module
    /* ---- Enable write protection ---- */
    SYSTEM.PRCR.WORD = 0xA500;              /* Disable writing to registers */
                                            /* related to the clock generation circuit. */
                                            /* Disable writing to registers */
                                            /* related to the low power consumption function. */
}

static inline void rx_ethernet_enable(void) {
    SYSTEM.PRCR.WORD = 0xA502;          /* protect off */
    SYSTEM.MSTPCRB.BIT.MSTPB15 = 0;     /* EtherC, EDMAC */
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
    ET_MDIO       : PA3
    ET_MDC        : PA4
    ET_LINKSTA    : PA5
    RMII_RXD1     : PB0
    RMII_RXD0     : PB1
    REF50CK       : PB2
    RMII_RX_ER    : PB3
    RMII_TXD_EN   : PB4
    RMII_TXD0     : PB5
    RMII_TXD1     : PB6
    RMII_CRS_DV   : PB7
    */
    #if defined(OP_OPTIMIZE)
    PORTA.PDR.BYTE &= ~0x38;
    PORTA.PDR.BYTE = 0;
    PORTA.PMR.BYTE &= ~0x38;
    PORTA.PMR.BYTE = 0;
    #else
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
    #endif
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
    MPC.PFENET.BIT.PHYMODE = 0; /* RMII mode */
    /* Switch to the selected input/output function */
    #if defined(OP_OPTIMIZE)
    PORTA.PMR.BYTE |= 0x38;
    PORTA.PMR.BYTE = 0xff;
    #else
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
    #endif
}

void HardwareSetup(void) {
    clock_init();
    rx_ethernet_enable();
    rx_ethernet_RMII_mode();
}
