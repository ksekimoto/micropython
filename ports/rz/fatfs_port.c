/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2019 Kentaro Sekimoto
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

#include "py/runtime.h"
#include "lib/oofatfs/ff.h"
#if MICROPY_HW_ENABLE_RTC
#include "rtc.h"
#endif
#include "rz_rtc.h"

MP_WEAK DWORD get_fattime(void) {
    #if MICROPY_HW_ENABLE_RTC
    rtc_init_finalise();
    int year = rz_rtc_get_year();
    int month = rz_rtc_get_month();
    int date = rz_rtc_get_date();
    int hour = rz_rtc_get_hour();
    int minute = rz_rtc_get_minute();
    int second = rz_rtc_get_second();
    return ((2000 + year - 1980) << 25) | ((month) << 21) | ((date) << 16) | ((hour) << 11) | ((minute) << 5) | (second / 2);
    #else
    // Jan 1st, 2018 at midnight. Not sure what timezone.
    return ((2018 - 1980) << 25) | ((1) << 21) | ((1) << 16) | ((0) << 11) | ((0) << 5) | (0 / 2);
    #endif
}
