/***************************************************************************
 *
 * PURPOSE
 *   RTC(Real Time Clock) function module file.
 *
 * TARGET DEVICE
 *   RX63N
 *
 * AUTHOR
 *   Renesas Electronics Corp.
 *
 *
 ***************************************************************************
 * Copyright (C) 2014 Renesas Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 ***************************************************************************/
/**
 * @file  RTC.cpp
 * @brief RX63Nマイコン内蔵の時計機能（RTC：リアル・タイム・クロック）を使うためのライブラリです。
 *
 * RTCクラスはこのライブラリをC++でカプセル化して使いやすくしたものです。
 *
 * Modified 27th May 2014 by Yuuki Okamiya from RL78duino.cpp
 */

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
#include <stdlib.h>
#include "iodefine.h"
#include "common.h"
#include "rx63n_rtc.h"

rx_rtc_cb_t rx_rtc_func = NULL;

static inline uint8_t int_to_bcd(int num) {
    return ((num / 10) << 4) | (num % 10);
}

static inline int bcd_to_int(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

int rx_rtc_get_year(void) {
    return bcd_to_int(RTC.RYRCNT.WORD) + 2000;
}

int rx_rtc_get_month(void) {
    return bcd_to_int(RTC.RMONCNT.BYTE);
}

int rx_rtc_get_date(void) {
    return bcd_to_int(RTC.RDAYCNT.BYTE);
}

int rx_rtc_get_hour(void) {
    return bcd_to_int(0x3f & RTC.RHRCNT.BYTE);
}

int rx_rtc_get_minute(void) {
    return bcd_to_int(RTC.RMINCNT.BYTE);
}

int rx_rtc_get_second(void) {
    return bcd_to_int(RTC.RSECCNT.BYTE);
}

int rx_rtc_get_weekday(void) {
    return bcd_to_int(RTC.RWKCNT.BYTE);
}

void rx_rtc_alarm_on() {
    /* Enable alarm and periodic interrupts*/
    RTC.RCR1.BIT.AIE = 1;
    while (!RTC.RCR1.BIT.AIE) { ; }
}

void rx_rtc_alarm_off() {
    /* Disable alarm and periodic interrupts*/
    RTC.RCR1.BIT.AIE = 0;
    while (RTC.RCR1.BIT.AIE) { ; }
}


void rx_rtc_set_alarm_time(int hour, int min, int week_flag) {
    /* Configure the alarm as follows -
     Alarm time - 12:00:00
     Enable the hour, minutes and seconds alarm      */
    RTC.RMINAR.BYTE = int_to_bcd(min);
    RTC.RHRAR.BYTE = int_to_bcd(hour);
    if (week_flag <= 0x06) {
        RTC.RWKAR.BYTE = week_flag;
    }
    RTC.RMINAR.BIT.ENB = 1;
    RTC.RHRAR.BIT.ENB = 1;
    if (week_flag <= 0x06) {
        RTC.RWKAR.BIT.ENB = 1;
    } else {
        RTC.RWKAR.BIT.ENB = 0;
    }
    /* Enable alarm and interrupts*/
    rx_rtc_alarm_on();
    /* Enable RTC Alarm interrupts */
    IPR(RTC, ALM)= 3u;
    IEN(RTC, ALM)= 1u;
    IR(RTC, ALM)= 0u;
}

/*
 * adj: adjustment bit (number of sub clocks)
 *   0 : no adjustment
 *   0<:
 *   0>:
 * aadjp
 *   0: 1 minute (RTC_PERIOD_MINUTE)
 *   1: 10 seconds (RTC_PERIOD_SECOND)
 */
void rx_rtc_correct(int adj, int aadjp) {
    int tmp_int;
    if (adj == 0) {
        RTC.RADJ.BYTE = 0x00;
        while (RTC.RADJ.BYTE != 0x00) { ; }
    } else if (adj > 0) {
        RTC.RADJ.BYTE = 0x00;
        while (RTC.RADJ.BYTE != 0x00) { ; }
        /* enable adjustment */
        RTC.RCR2.BIT.AADJE = 1;
        while (RTC.RCR2.BIT.AADJE != 1) { ; }
        RTC.RCR2.BIT.AADJP =
            aadjp == RTC_PERIOD_MINUTE ? RTC_PERIOD_MINUTE : RTC_PERIOD_SECOND;
        while (RTC.RCR2.BIT.AADJP != 1) { ; }
        tmp_int = 0x40 | (0x3F & adj);  /* 0x40 + */
        RTC.RADJ.BYTE = tmp_int;
        while (RTC.RADJ.BYTE != tmp_int) { ; }
    } else {
        RTC.RADJ.BYTE = 0x00;
        while (RTC.RADJ.BYTE != 0x00) { ; }
        /* enable adjustment */
        RTC.RCR2.BIT.AADJE = 1;
        while (RTC.RCR2.BIT.AADJE != 1) { ; }
        RTC.RCR2.BIT.AADJP =
            aadjp == RTC_PERIOD_MINUTE ? RTC_PERIOD_MINUTE : RTC_PERIOD_SECOND;
        while (RTC.RCR2.BIT.AADJP != 1) { ; }
        tmp_int = 0x80 | (0x3F & abs(adj)); /* 0x80 - */
        RTC.RADJ.BYTE = tmp_int;
        while (RTC.RADJ.BYTE != tmp_int) { ; }
    }
}

void rx_rtc_set_time(rtc_t *time) {
    /* Write 0 to RTC start bit */
    RTC.RCR2.BIT.START = 0x0;
    /* Wait for start bit to clear */
    while (0 != RTC.RCR2.BIT.START) { ; }
    /* Alarm enable bits are undefined after a reset,
     disable non-required alarm features */
    RTC.RWKAR.BIT.ENB = 0;
    RTC.RDAYAR.BIT.ENB = 0;
    RTC.RMONAR.BIT.ENB = 0;
    RTC.RYRAREN.BIT.ENB = 0;
    /* Operate RTC in 24-hr mode */
    RTC.RCR2.BIT.HR24 = 0x1;
    RTC.RYRCNT.WORD = int_to_bcd(time->year % 100);
    RTC.RMONCNT.BYTE = int_to_bcd(time->month);
    RTC.RDAYCNT.BYTE = int_to_bcd(time->date);
    RTC.RHRCNT.BYTE = int_to_bcd(time->hour);
    RTC.RMINCNT.BYTE = int_to_bcd(time->minute);
    RTC.RSECCNT.BYTE = int_to_bcd(time->second);
    RTC.RWKCNT.BYTE = int_to_bcd(time->weekday);
    /* Start the clock */
    RTC.RCR2.BIT.START = 0x1;
    /* Wait until the start bit is set to 1 */
    while (1 != RTC.RCR2.BIT.START) { ; }
}

void rx_rtc_get_time(rtc_t *time) {
#if defined(RX63N)
    IEN(RTC, CUP)= 0;
    RTC.RCR1.BIT.CIE = 1;
    do {
        IR(RTC, CUP) = 0;
        time->year = bcd_to_int(RTC.RYRCNT.WORD) + 2000;
        time->month = bcd_to_int(RTC.RMONCNT.BYTE);
        time->date = bcd_to_int(RTC.RDAYCNT.BYTE);
        time->hour = bcd_to_int(0x3f & RTC.RHRCNT.BYTE);
        time->minute = bcd_to_int(RTC.RMINCNT.BYTE);
        time->second = bcd_to_int(RTC.RSECCNT.BYTE);
        time->weekday = bcd_to_int(RTC.RWKCNT.BYTE);
    } while (IR(RTC, CUP));
    RTC.RCR1.BIT.CIE = 0;
#endif
#if defined(RX65N)
    // ToDo: implement interrupt configuration
    RTC.RCR1.BIT.CIE = 1;
    time->year = bcd_to_int(RTC.RYRCNT.WORD) + 2000;
    time->month = bcd_to_int(RTC.RMONCNT.BYTE);
    time->date = bcd_to_int(RTC.RDAYCNT.BYTE);
    time->hour = bcd_to_int(0x3f & RTC.RHRCNT.BYTE);
    time->minute = bcd_to_int(RTC.RMINCNT.BYTE);
    time->second = bcd_to_int(RTC.RSECCNT.BYTE);
    time->weekday = bcd_to_int(RTC.RWKCNT.BYTE);
    RTC.RCR1.BIT.CIE = 0;
#endif
}

static void wait(volatile int count) {
    while (count--) {
        __asm__ __volatile__ ("nop");
    }
}

void rx_rtc_init(void) {
    SYSTEM.PRCR.WORD = 0xA503;
    /* Check if the MCU has come from a cold start (power on reset) */
    if (0 == SYSTEM.RSTSR1.BIT.CWSF) {
        /* Set the warm start flag */
        SYSTEM.RSTSR1.BIT.CWSF = 1;
        /* Disable the sub-clock oscillator */
        SYSTEM.SOSCCR.BIT.SOSTP = 1;
        /* Wait for register modification to complete */
        while (1 != SYSTEM.SOSCCR.BIT.SOSTP) { ; }
        /* Disable the input from the sub-clock */
        RTC.RCR3.BYTE = 0x0C;
        /* Wait for the register modification to complete */
        while (0 != RTC.RCR3.BIT.RTCEN) { ; }
        /* Wait for at least 5 cycles of sub-clock */
        wait(0x1000);
        /* Start sub-clock */
        SYSTEM.SOSCCR.BIT.SOSTP = 0;
        /* Perform 8 delay iterations */
        for (uint8_t i = 0; i < 8; i++) {
            /* Wait in while loop for ~0.5 seconds */
            wait(0xFFFFE);
        }
    } else {
        /* Start sub-clock */
        SYSTEM.SOSCCR.BIT.SOSTP = 0;
        /* Wait for the register modification to complete */
        while (0 != SYSTEM.SOSCCR.BIT.SOSTP) { ; }
    }
    /* Set RTC clock input from sub-clock, and supply to RTC module */
    RTC.RCR4.BIT.RCKSEL = 0;
    RTC.RCR3.BIT.RTCEN = 1;
    /* Wait for at least 5 cycles of sub-clock */
    wait(0x1000);
    /* It is now safe to set the RTC registers */
    /* Stop the clock */
    RTC.RCR2.BIT.START = 0x0;
    /* Wait for start bit to clear */
    while (0 != RTC.RCR2.BIT.START) { ; }
    /* Reset the RTC unit */
    RTC.RCR2.BIT.RESET = 1;
    /* Wait until reset is complete */
    while (RTC.RCR2.BIT.RESET) { ; }
    /* call back */
    rx_rtc_func = NULL;
}

void rx_rtc_deinit(void) {
    RTC.RCR3.BIT.RTCEN = 0;
    RTC.RCR4.BIT.RCKSEL = 1;
    rx_rtc_func = NULL;
}
