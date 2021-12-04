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
 * @file  RTC1.cpp
 * Modified 27th May 2014 by Yuuki Okamiya from RL78duino.cpp
 */

/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include <stdlib.h>
#include "iodefine.h"
#include "rtc_iodefine.h"
#include "common.h"
#include "rz_rtc.h"

#define USE_EXTAL_CLK
#define READ_LOOP_MAX    (2000)

rz_rtc_cb_t rz_rtc_func = NULL;

static inline uint8_t int_to_bcd(int num) {
    return ((num / 10) << 4) | (num % 10);
}

static inline int bcd_to_int(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

int rz_rtc_get_year(void) {
    return bcd_to_int(RTC1.RYRCNT.WORD) + 2000;
}

int rz_rtc_get_month(void) {
    return bcd_to_int(RTC1.RMONCNT.BYTE);
}

int rz_rtc_get_date(void) {
    return bcd_to_int(RTC1.RDAYCNT.BYTE);
}

int rz_rtc_get_hour(void) {
    return bcd_to_int(0x3f & RTC1.RHRCNT.BYTE);
}

int rz_rtc_get_minute(void) {
    return bcd_to_int(RTC1.RMINCNT.BYTE);
}

int rz_rtc_get_second(void) {
    return bcd_to_int(RTC1.RSECCNT.BYTE);
}

int rz_rtc_get_weekday(void) {
    return bcd_to_int(RTC1.RWKCNT.BYTE);
}

void rz_rtc_alarm_on() {
    /* Enable alarm and periodic interrupts*/
    RTC1.RCR1.BIT.AIE = 1;
    while (!RTC1.RCR1.BIT.AIE) {
        ;
    }
}

void rz_rtc_alarm_off() {
    /* Disable alarm and periodic interrupts*/
    RTC1.RCR1.BIT.AIE = 0;
    while (RTC1.RCR1.BIT.AIE) {
        ;
    }
}


void rz_rtc_set_alarm_time(int hour, int min, int week_flag) {
    /* Configure the alarm as follows -
     Alarm time - 12:00:00
     Enable the hour, minutes and seconds alarm      */
    RTC1.RMINAR.BYTE = int_to_bcd(min);
    RTC1.RHRAR.BYTE = int_to_bcd(hour);
    if (week_flag <= 0x06) {
        RTC1.RWKAR.BYTE = week_flag;
    }
    RTC1.RMINAR.BIT.ENB = 1;
    RTC1.RHRAR.BIT.ENB = 1;
    if (week_flag <= 0x06) {
        RTC1.RWKAR.BIT.ENB = 1;
    } else {
        RTC1.RWKAR.BIT.ENB = 0;
    }
    /* Enable alarm and interrupts*/
    rz_rtc_alarm_on();
    /* Enable RTC Alarm interrupts */
    // IPR(RTC, ALM)= 3u;
    // IEN(RTC, ALM)= 1u;
    // IR(RTC, ALM)= 0u;
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
void rz_rtc_correct(int adj, int aadjp) {
    int tmp_int;
    if (adj == 0) {
        RTC1.RADJ.BYTE = 0x00;
        while (RTC1.RADJ.BYTE != 0x00) {
            ;
        }
    } else if (adj > 0) {
        RTC1.RADJ.BYTE = 0x00;
        while (RTC1.RADJ.BYTE != 0x00) {
            ;
        }
        /* enable adjustment */
        RTC1.RCR2.BIT.AADJE = 1;
        while (RTC1.RCR2.BIT.AADJE != 1) {
            ;
        }
        RTC1.RCR2.BIT.AADJP =
            aadjp == RTC_PERIOD_MINUTE ? RTC_PERIOD_MINUTE : RTC_PERIOD_SECOND;
        while (RTC1.RCR2.BIT.AADJP != 1) {
            ;
        }
        tmp_int = 0x40 | (0x3F & adj);  /* 0x40 + */
        RTC1.RADJ.BYTE = tmp_int;
        while (RTC1.RADJ.BYTE != tmp_int) {
            ;
        }
    } else {
        RTC1.RADJ.BYTE = 0x00;
        while (RTC1.RADJ.BYTE != 0x00) {
            ;
        }
        /* enable adjustment */
        RTC1.RCR2.BIT.AADJE = 1;
        while (RTC1.RCR2.BIT.AADJE != 1) {
            ;
        }
        RTC1.RCR2.BIT.AADJP =
            aadjp == RTC_PERIOD_MINUTE ? RTC_PERIOD_MINUTE : RTC_PERIOD_SECOND;
        while (RTC1.RCR2.BIT.AADJP != 1) {
            ;
        }
        tmp_int = 0x80 | (0x3F & abs(adj)); /* 0x80 - */
        RTC1.RADJ.BYTE = tmp_int;
        while (RTC1.RADJ.BYTE != tmp_int) {
            ;
        }
    }
}

void rz_rtc_set_time(rtc_t *time) {
    /* Write 0 to RTC start bit */
    RTC1.RCR2.BIT.START = 0x0;
    /* Wait for start bit to clear */
    while (0 != RTC1.RCR2.BIT.START) {
        ;
    }
    /* Alarm enable bits are undefined after a reset,
     disable non-required alarm features */
    RTC1.RWKAR.BIT.ENB = 0;
    RTC1.RDAYAR.BIT.ENB = 0;
    RTC1.RMONAR.BIT.ENB = 0;
    RTC1.RYRAREN.BIT.ENB = 0;
    /* Operate RTC in 24-hr mode */
    RTC1.RCR2.BIT.HR24 = 0x1;
    RTC1.RYRCNT.WORD = int_to_bcd(time->year % 100);
    RTC1.RMONCNT.BYTE = int_to_bcd(time->month);
    RTC1.RDAYCNT.BYTE = int_to_bcd(time->date);
    RTC1.RHRCNT.BYTE = int_to_bcd(time->hour);
    RTC1.RMINCNT.BYTE = int_to_bcd(time->minute);
    RTC1.RSECCNT.BYTE = int_to_bcd(time->second);
    RTC1.RWKCNT.BYTE = int_to_bcd(time->weekday);
    /* Start the clock */
    RTC1.RCR2.BIT.START = 0x1;
    /* Wait until the start bit is set to 1 */
    while (1 != RTC1.RCR2.BIT.START) {
        ;
    }
}

void rz_rtc_get_time(rtc_t *time) {
    // ToDo: implement interrupt configuration
    RTC1.RCR1.BIT.CIE = 1;
    time->year = bcd_to_int(RTC1.RYRCNT.WORD) + 2000;
    time->month = bcd_to_int(RTC1.RMONCNT.BYTE);
    time->date = bcd_to_int(RTC1.RDAYCNT.BYTE);
    time->hour = bcd_to_int(0x3f & RTC1.RHRCNT.BYTE);
    time->minute = bcd_to_int(RTC1.RMINCNT.BYTE);
    time->second = bcd_to_int(RTC1.RSECCNT.BYTE);
    time->weekday = bcd_to_int(RTC1.RWKCNT.BYTE);
    RTC1.RCR1.BIT.CIE = 0;
}

#if RZ_TODO
static void wait(volatile int count) {
    while (count--) {
        __asm__ __volatile__ ("nop");
    }
}
#endif

void rz_rtc_init(void) {
    volatile int i;
    CPG.STBCR5.BIT.MSTP52 = 0;
    #if defined(USE_RTCX1_CLK)
    RTC1.RCR4.BIT.RCKSEL = 0;
    RTC1.RCR3.BIT.RTCEN = 1;
    #elif defined(USE_EXTAL_CLK)
    RTC1.RCR4.BIT.RCKSEL = 1;
    RTC1.RCR3.BIT.RTCEN = 0;
    #endif
    i = 0;
    while (i < 1000) {
        i++;
    }

    RTC_BCNT1.RCR2.BIT.START = 0;
    for (i = 0; (i < READ_LOOP_MAX) && (RTC1.RCR2.BIT.START != 0); i++) {
        ;
    }
    #if defined(USE_EXTAL_CLK)
    // Clockin  = 24MHz
    RTC1.RFRH.WORD = 0x0001;
    RTC1.RFRL.WORD = 0x6E35;
    #endif

    RTC1.RCR2.BIT.CNTMD = 1;
    for (i = 0; (i < READ_LOOP_MAX) && (RTC1.RCR2.BIT.CNTMD != 1); i++) {
        ;
    }

    RTC1.RCR2.BIT.RESET = 1;
    for (i = 0; (i < READ_LOOP_MAX) && (RTC1.RCR2.BIT.RESET != 0); i++) {
        ;
    }

    RTC1.RCR1.BIT.AIE = 1;
    RTC1.RCR1.BIT.PIE = 0;

    RTC1.RCR2.BIT.START = 1;
    for (i = 0; (i < READ_LOOP_MAX) && (RTC1.RCR2.BIT.START != 1); i++) {
        ;
    }
}

void rz_rtc_deinit(void) {
    rz_rtc_func = NULL;
}
