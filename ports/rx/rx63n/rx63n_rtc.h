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

#ifndef RX63N_RTC_H_
#define RX63N_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_PERIOD_MINUTE   0x00
#define RTC_PERIOD_SECOND   0x01

typedef struct {
    unsigned short  year;
    unsigned char   month;
    unsigned char   date;
    unsigned char   weekday;
    unsigned char   hour;
    unsigned char   minute;
    unsigned char   second;
} rtc_t;

typedef void (*rx_rtc_cb_t)(void);

int rx_rtc_get_year(void);
int rx_rtc_get_month(void);
int rx_rtc_get_date(void);
int rx_rtc_get_hour(void);
int rx_rtc_get_minute(void);
int rx_rtc_get_second(void);
int rx_rtc_get_weekday(void);
void rx_rtc_init(void);
void rx_rtc_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RX63N_RTC_H_ */
