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

#include <stdio.h>
#include <stdint.h>
#include "iodefine.h"

#if defined(GRROSE)
static inline void rx_ethernet_enable(void)
{
    SYSTEM.PRCR.WORD = 0xA502;          /* protect off */
    SYSTEM.MSTPCRB.BIT.MSTPB15 = 0;     /* EtherC, EDMAC */
    //BSC.BEREN.BIT.TOEN = 1;
    SYSTEM.PRCR.WORD = 0xA500;          /* protect on */
}

static inline void rx_ethernet_disable(void)
{
    SYSTEM.PRCR.WORD = 0xA502;          /* protect off */
    SYSTEM.MSTPCRB.BIT.MSTPB15 = 1;     /* EtherC, EDMAC */
    SYSTEM.PRCR.WORD = 0xA500;          /* protect on */
}

static inline void rx_ethernet_RMII_mode(void)
{
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
#endif

#if defined(RX63N)
static void clock_init(void) {
    volatile int i;
    SYSTEM.PRCR.WORD = 0xA503;              // Protect register (protect off)
    if (SYSTEM.RSTSR0.BIT.DPSRSTF == 1) {   // Reset status register (Deep software standby reset check)
        if (SYSTEM.DPSIFR2.BIT.DUSBIF == 1) { // Deep standby interrupt flag register2
            SYSTEM.DPSIFR2.BIT.DUSBIF = 0;  // Clear USB Request
        }
        PORT3.PMR.BIT.B6 = 1;               // Port mode register
        PORT3.PMR.BIT.B7 = 1;               // Port mode register
        SYSTEM.MOSCCR.BYTE = 0x00;          // Main clock oscillator is operated
        SYSTEM.MOSCWTCR.BYTE = 0x0D;        // 131072 state
        SYSTEM.SOSCCR.BYTE = 0x01;          // Sub clock Oscillator is stopped
        SYSTEM.PLLCR.WORD = 0x0F00;         // PLIDIV = 12MHz(/1), STC = 192MHz(*16)
        SYSTEM.PLLCR2.BYTE = 0x00;          // PLL enable
        //SYSTEM.PLLWTCR.BYTE = 0x0F;         // 4194304cycle(Default)
        for (i = 0; i < 600; i++) {
        }
        SYSTEM.SCKCR.LONG = 0x21032222;     // ICK(96MHz)=PLL/2,BCK(24MHz)=PLL/8,FCK,PCK(48MHz)=PLL/4
        SYSTEM.SCKCR3.WORD = 0x0400;        // PLL
        SYSTEM.SCKCR2.BIT.UCK = 3;          // USB clock : 48MHz
        SYSTEM.MSTPCRA.LONG = 0x7FFFFFFF;   // Module stop control register (Disable ACSE)
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
        //SYSTEM.PLLWTCR.BYTE = 0x0F;
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

void bootstrap(void) {
    //SYSTEM.SCKCR.LONG = 0x21032200;     /* clock init: ICK=PLL/2, BCLK=PLL/8, PCLK=PLL/4 */
    clock_init();
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

#endif

#if defined(RX65N)
/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2016 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : resetprg.c
* Device(s)    : RX65N
* Description  : Defines post-reset routines that are used to configure the MCU prior to the main program starting.
*                This is were the program counter starts on power-up or reset.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version   Description
*         : 01.10.2016 1.00      First Release
*         : 15.05.2017 2.00      Deleted unnecessary comments.
*                                Applied the following technical update.
*                                - TN-RX*-A164A - Added initialization procedure when the realtime clock
*                                                 is not to be used.
*                                Changed the sub-clock oscillator settings.
*                                Added the bsp startup module disable function.
*         : 01.07.2018 2.01      Added support for RTOS.
*                                Added processing after writing ROMWT register.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

#define ROMWT_FREQ_THRESHOLD_01 50000000         // ICLK > 50MHz requires ROMWT register update
#define ROMWT_FREQ_THRESHOLD_02 100000000        // ICLK > 100MHz requires ROMWT register update

#define BSP_CFG_MAIN_CLOCK_SOURCE       (0) //    0 = Resonator, 1 = External clock input
/* Clock source select (CKSEL).
   0 = Low Speed On-Chip Oscillator  (LOCO)
   1 = High Speed On-Chip Oscillator (HOCO)
   2 = Main Clock Oscillator
   3 = Sub-Clock Oscillator
   4 = PLL Circuit
*/
#define BSP_CFG_CLOCK_SOURCE        (4)
#define BSP_CFG_PLL_SRC             (0)
#define BSP_CFG_XTAL_HZ             (12000000)
#define BSP_CFG_MAIN_CLOCK_SOURCE   (0)
#define BSP_CFG_MOSC_WAIT_TIME      (0x53)
#define BSP_CFG_PLL_DIV             (1)
#define BSP_CFG_PLL_MUL             (20.0)
#define BSP_CFG_ICK_DIV             (2)
#define BSP_SELECTED_CLOCK_HZ       ((BSP_CFG_XTAL_HZ/BSP_CFG_PLL_DIV) * BSP_CFG_PLL_MUL)
#define BSP_ICLK_HZ                 (BSP_SELECTED_CLOCK_HZ / BSP_CFG_ICK_DIV)

static void clock_source_select (void)
{
    volatile uint8_t i;
    volatile uint8_t dummy;

    SYSTEM.MOFCR.BIT.MOFXIN = 0;
    SYSTEM.MOFCR.BIT.MOSEL = BSP_CFG_MAIN_CLOCK_SOURCE;

    /* If HOCO is already operating, it doesn't stop.  */
    if (1 == SYSTEM.HOCOCR.BIT.HCSTP) {
        SYSTEM.HOCOPCR.BYTE = 0x01; /* Turn off power to HOCO. */
    } else {
        while (0 == SYSTEM.OSCOVFSR.BIT.HCOVF) {
            /* The delay period needed is to make sure that the HOCO has stabilized. */
        }
    }

    if (BSP_CFG_XTAL_HZ > 20000000) {           /* 20 - 24MHz. */
        SYSTEM.MOFCR.BIT.MODRV2 = 0;
    } else if (BSP_CFG_XTAL_HZ > 16000000) {    /* 16 - 20MHz. */
        SYSTEM.MOFCR.BIT.MODRV2 = 1;
    } else if (BSP_CFG_XTAL_HZ > 8000000) {     /* 8 - 16MHz. */
        SYSTEM.MOFCR.BIT.MODRV2 = 2;
    } else  {                                   /* 8MHz. */
        SYSTEM.MOFCR.BIT.MODRV2 = 3;
    }

    SYSTEM.MOSCWTCR.BYTE = BSP_CFG_MOSC_WAIT_TIME;
    SYSTEM.MOSCCR.BYTE = 0x00;

    if (0x00 ==  SYSTEM.MOSCCR.BYTE) {  /* Dummy read */
        __asm("nop");
    }
    while (0 == SYSTEM.OSCOVFSR.BIT.MOOVF) {    /* wait for stable */
    }
    if (0 == SYSTEM.RSTSR1.BIT.CWSF) {
        /* Cold start setting */
        RTC.RCR4.BIT.RCKSEL = 0;
        for (i = 0; i < 4; i++) {
            dummy = RTC.RCR4.BYTE;
            dummy;
        }
        if (0 != RTC.RCR4.BIT.RCKSEL) {
            __asm("nop");
        }
        RTC.RCR3.BIT.RTCEN = 0;
        for (i = 0; i < 4; i++) {
            dummy = RTC.RCR3.BYTE;
            dummy;
        }
        if (0 != RTC.RCR3.BIT.RTCEN) {
            __asm("nop");
        }
        SYSTEM.SOSCCR.BYTE = 0x01;
        if (0x01 != SYSTEM.SOSCCR.BYTE) {
            __asm("nop");
        }
        while (0 != SYSTEM.OSCOVFSR.BIT.SOOVF) {
        }
    } else
    {
        SYSTEM.SOSCCR.BYTE = 0x01;
        if (0x01 != SYSTEM.SOSCCR.BYTE) {
            __asm("nop");
        }
        while (0 != SYSTEM.OSCOVFSR.BIT.SOOVF) {
        }
    }

    /* Set PLL Input Divisor. */
    SYSTEM.PLLCR.BIT.PLIDIV = BSP_CFG_PLL_DIV - 1;
    #if (BSP_CFG_PLL_SRC == 0)
    SYSTEM.PLLCR.BIT.PLLSRCSEL = 0;
    #else
    SYSTEM.PLLCR.BIT.PLLSRCSEL = 1;
    #endif
    SYSTEM.PLLCR.BIT.STC = ((uint8_t)((float)BSP_CFG_PLL_MUL * 2.0)) - 1;
    SYSTEM.PLLCR2.BYTE = 0x00;
    while(0 == SYSTEM.OSCOVFSR.BIT.PLOVF) {
    }
#if (BSP_CFG_CLOCK_SOURCE == 0)
    SYSTEM.LOCOCR.BYTE = 0x00;
#endif
    /* RX65N has a ROMWT register which controls the cycle waiting for access to code flash memory.
       It is set as zero coming out of reset.
       When setting ICLK to [50 MHz < ICLK <= 100 MHz], set the ROMWT.ROMWT[1:0] bits to 01b.
       When setting ICLK to [100 MHz < ICLK <= 120 MHz], set the ROMWT.ROMWT[1:0] bits to 10b. */
    if (BSP_ICLK_HZ > ROMWT_FREQ_THRESHOLD_02) {
        SYSTEM.ROMWT.BYTE = 0x02;
        /* Dummy read and compare. cf."5. I/O Registers", "(2) Notes on writing to I/O registers" in User's manual. */
        if (0x02 == SYSTEM.ROMWT.BYTE) {
            __asm("nop");
        }
    } else if (BSP_ICLK_HZ > ROMWT_FREQ_THRESHOLD_01) {
        SYSTEM.ROMWT.BYTE = 0x01;
        if (0x01 == SYSTEM.ROMWT.BYTE) {
            __asm("nop");
        }
    } else {
        /* Do nothing. */
    }
}

/*
Default settings:
Clock Description              Frequency
----------------------------------------
Input Clock Frequency............  24 MHz
PLL frequency (x10).............. 240 MHz
Internal Clock Frequency......... 120 MHz
Peripheral Clock Frequency A..... 120 MHz
Peripheral Clock Frequency B.....  60 MHz
Peripheral Clock Frequency C.....  60 MHz
Peripheral Clock Frequency D.....  60 MHz
Flash IF Clock Frequency.........  60 MHz
External Bus Clock Frequency..... 120 MHz
USB Clock Frequency..............  48 MHz */

#define BSP_CFG_CLOCK_SOURCE            (4) // PLL
#define BSP_CFG_ROM_CACHE_ENABLE        (0) // disable rom cache

static void clock_init(void) {

    SYSTEM.PRCR.WORD = 0xA50B;
    clock_source_select();

    uint32_t temp_clock = 0;
    temp_clock |= 0x20000000;   // FCK bits DIV=4
    temp_clock |= 0x01000000;   // ICK bits DIV=2
    temp_clock |= 0x00010000;   // BCK bits DIV=2
    temp_clock |= 0x00800000;   // BCLK no output
    temp_clock |= 0x00400000;   // SDCLK no output
    temp_clock |= 0x00001000;   // PCLKA DIV=2
    temp_clock |= 0x00000200;   // PCLKB DIV=4
    temp_clock |= 0x00000020;   // PCLKC DIV=4
    temp_clock |= 0x00000002;   // PCLKD DIV=4
    SYSTEM.SCKCR.LONG = temp_clock;

    temp_clock = 0;
    temp_clock |= 0x00000041;	// DIV=5
    SYSTEM.SCKCR2.WORD = (uint16_t)temp_clock;

    SYSTEM.SCKCR3.WORD = ((uint16_t)BSP_CFG_CLOCK_SOURCE) << 8; // PLL
#if (BSP_CFG_CLOCK_SOURCE != 0)
    SYSTEM.LOCOCR.BYTE = 0x01;  // LOCO off
#endif

#if (BSP_CFG_ROM_CACHE_ENABLE == 1)
    FLASH.ROMCIV.WORD = 0x0001;
    while (FLASH.ROMCIV.WORD != 0x0000) {
    }
    FLASH.ROMCE.WORD = 0x0001;
    while (FLASH.ROMCE.WORD != 0x0001) {
    }
#endif
    SYSTEM.MSTPCRB.BIT.MSTPB19 = 0u;
    SYSTEM.PRCR.WORD = 0xA500;
}

void bootstrap(void) {
    clock_init();
    /* Initialize MCU interrupt callbacks. */
    // bsp_interrupt_open();
    /* Initialize register protection functionality. */
    // bsp_register_protect_open();
#if defined(GRROSE)
    rx_ethernet_enable();
    rx_ethernet_RMII_mode();
#endif
}

#endif
