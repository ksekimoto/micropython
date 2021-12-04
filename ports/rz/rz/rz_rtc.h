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

#ifndef RZ_RZ_RTC_H_
#define RZ_RZ_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_PERIOD_MINUTE   0x00
#define RTC_PERIOD_SECOND   0x01

typedef struct {
    unsigned short year;
    unsigned char month;
    unsigned char date;
    unsigned char weekday;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} rtc_t;

typedef void (*rz_rtc_cb_t)(void);

int rz_rtc_get_year(void);
int rz_rtc_get_month(void);
int rz_rtc_get_date(void);
int rz_rtc_get_hour(void);
int rz_rtc_get_minute(void);
int rz_rtc_get_second(void);
int rz_rtc_get_weekday(void);
void rz_rtc_init(void);
void rz_rtc_deinit(void);
void rz_rtc_set_time(rtc_t *time);
void rz_rtc_get_time(rtc_t *time);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_RTC_H_ */
