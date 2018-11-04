/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include "py/runtime.h"
#include "extint.h"
#include "rtc.h"
#include "irq.h"

/// \moduleref pyb
/// \class RTC - real time clock
///
/// The RTC is and independent clock that keeps track of the date
/// and time.
///
/// Example usage:
///
///     rtc = pyb.RTC()
///     rtc.datetime((2014, 5, 1, 4, 13, 0, 0, 0))
///     print(rtc.datetime())

// rtc_info indicates various things about RTC startup
// it's a bit of a hack at the moment
static mp_uint_t rtc_info;

// Note: LSI is around (32KHz), these dividers should work either way
// ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)
// modify RTC_ASYNCH_PREDIV & RTC_SYNCH_PREDIV in board/<BN>/mpconfigport.h to change sub-second ticks
// default is 3906.25 µs, min is ~30.52 µs (will increas Ivbat by ~500nA)
#ifndef RTC_ASYNCH_PREDIV
#define RTC_ASYNCH_PREDIV (0x7f)
#endif
#ifndef RTC_SYNCH_PREDIV
#define RTC_SYNCH_PREDIV  (0x00ff)
#endif

#if defined(MICROPY_HW_RTC_USE_LSE) && MICROPY_HW_RTC_USE_LSE
STATIC bool rtc_use_lse = true;
#else
STATIC bool rtc_use_lse = false;
#endif
STATIC uint32_t rtc_startup_tick;
STATIC bool rtc_need_init_finalise = false;

// check if LSE exists
// not well tested, should probably be removed
STATIC bool lse_magic(void) {
#if 0
    uint32_t mode_in = GPIOC->MODER & 0x3fffffff;
    uint32_t mode_out = mode_in | 0x40000000;
    GPIOC->MODER = mode_out;
    GPIOC->OTYPER &= 0x7fff;
    GPIOC->BSRRH = 0x8000;
    GPIOC->OSPEEDR &= 0x3fffffff;
    GPIOC->PUPDR &= 0x3fffffff;
    int i = 0xff0;
    __IO int d = 0;
    uint32_t tc = 0;
    __IO uint32_t j;
    while (i) {
        GPIOC->MODER = mode_out;
        GPIOC->MODER = mode_in;
        for (j = 0; j < d; j++) ;
        i--;
        if ((GPIOC->IDR & 0x8000) == 0) {
            tc++;
        }
    }
    return (tc < 0xff0)?true:false;
#else
    return false;
#endif
}

void rtc_init_start(bool force_init) {
    // ToDo
}

void rtc_init_finalise() {
    if (!rtc_need_init_finalise) {
        return;
    }
}

#define PYB_LSE_TIMEOUT_VALUE 1000  // ST docs spec 2000 ms LSE startup, seems to be too pessimistic
#define PYB_LSI_TIMEOUT_VALUE 500   // this is way too pessimistic, typ. < 1ms

/******************************************************************************/
// MicroPython bindings

typedef struct _pyb_rtc_obj_t {
    mp_obj_base_t base;
} pyb_rtc_obj_t;

STATIC const pyb_rtc_obj_t pyb_rtc_obj = {{&pyb_rtc_type}};

/// \classmethod \constructor()
/// Create an RTC object.
STATIC mp_obj_t pyb_rtc_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return constant object
    return MP_OBJ_FROM_PTR(&pyb_rtc_obj);
}

// force rtc to re-initialise
mp_obj_t pyb_rtc_init(mp_obj_t self_in) {
    rtc_init_start(true);
    rtc_init_finalise();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(pyb_rtc_init_obj, pyb_rtc_init);

/// \method info()
/// Get information about the startup time and reset source.
///
///  - The lower 0xffff are the number of milliseconds the RTC took to
///    start up.
///  - Bit 0x10000 is set if a power-on reset occurred.
///  - Bit 0x20000 is set if an external reset occurred
mp_obj_t pyb_rtc_info(mp_obj_t self_in) {
    return mp_obj_new_int(rtc_info);
}
MP_DEFINE_CONST_FUN_OBJ_1(pyb_rtc_info_obj, pyb_rtc_info);

/// \method datetime([datetimetuple])
/// Get or set the date and time of the RTC.
///
/// With no arguments, this method returns an 8-tuple with the current
/// date and time.  With 1 argument (being an 8-tuple) it sets the date
/// and time.
///
/// The 8-tuple has the following format:
///
///     (year, month, day, weekday, hours, minutes, seconds, subseconds)
///
/// `weekday` is 1-7 for Monday through Sunday.
///
/// `subseconds` counts down from 255 to 0

#define MEG_DIV_64 (1000000 / 64)
#define MEG_DIV_SCALE ((RTC_SYNCH_PREDIV + 1) / 64)

#if defined(MICROPY_HW_RTC_USE_US) && MICROPY_HW_RTC_USE_US
uint32_t rtc_subsec_to_us(uint32_t ss) {
    return ((RTC_SYNCH_PREDIV - ss) * MEG_DIV_64) / MEG_DIV_SCALE;
}

uint32_t rtc_us_to_subsec(uint32_t us) {
    return RTC_SYNCH_PREDIV - (us * MEG_DIV_SCALE / MEG_DIV_64);
}
#else
#define rtc_us_to_subsec
#define rtc_subsec_to_us
#endif

mp_obj_t pyb_rtc_datetime(size_t n_args, const mp_obj_t *args) {
    rtc_init_finalise();
    if (n_args == 1) {
        // get date and time
        // note: need to call get time then get date to correctly access the registers
        RTC_DateTypeDef date;
        RTC_TimeTypeDef time;
        //HAL_RTC_GetTime(&RTCHandle, &time, RTC_FORMAT_BIN);
        //HAL_RTC_GetDate(&RTCHandle, &date, RTC_FORMAT_BIN);
        mp_obj_t tuple[8] = {
            mp_obj_new_int(2000 + date.Year),
            mp_obj_new_int(date.Month),
            mp_obj_new_int(date.Date),
            mp_obj_new_int(date.WeekDay),
            mp_obj_new_int(time.Hours),
            mp_obj_new_int(time.Minutes),
            mp_obj_new_int(time.Seconds),
            mp_obj_new_int(rtc_subsec_to_us(time.SubSeconds)),
        };
        return mp_obj_new_tuple(8, tuple);
    } else {
        // set date and time
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[1], 8, &items);
        // ToDo
        return mp_const_none;
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_rtc_datetime_obj, 1, 2, pyb_rtc_datetime);

// wakeup(None)
// wakeup(ms, callback=None)
// wakeup(wucksel, wut, callback)
mp_obj_t pyb_rtc_wakeup(size_t n_args, const mp_obj_t *args) {
    // ToDo
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_rtc_wakeup_obj, 2, 4, pyb_rtc_wakeup);

// calibration(None)
// calibration(cal)
// When an integer argument is provided, check that it falls in the range [-511 to 512]
// and set the calibration value; otherwise return calibration value
mp_obj_t pyb_rtc_calibration(size_t n_args, const mp_obj_t *args) {
    // ToDo
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_rtc_calibration_obj, 1, 2, pyb_rtc_calibration);

STATIC const mp_rom_map_elem_t pyb_rtc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_rtc_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&pyb_rtc_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_datetime), MP_ROM_PTR(&pyb_rtc_datetime_obj) },
    { MP_ROM_QSTR(MP_QSTR_wakeup), MP_ROM_PTR(&pyb_rtc_wakeup_obj) },
    { MP_ROM_QSTR(MP_QSTR_calibration), MP_ROM_PTR(&pyb_rtc_calibration_obj) },
};
STATIC MP_DEFINE_CONST_DICT(pyb_rtc_locals_dict, pyb_rtc_locals_dict_table);

const mp_obj_type_t pyb_rtc_type = {
    { &mp_type_type },
    .name = MP_QSTR_RTC,
    .make_new = pyb_rtc_make_new,
    .locals_dict = (mp_obj_dict_t*)&pyb_rtc_locals_dict,
};
