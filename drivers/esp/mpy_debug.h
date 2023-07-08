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
#ifndef DRIVERS_ESP_MPY_DEBUG_H_
#define DRIVERS_ESP_MPY_DEBUG_H_

#include <stdio.h>
#include <stdint.h>

#define USE_MPY_DEBUG

#if defined(USE_SEGGER_RTT)
#include "SEGGER_RTT.h"
// volatile int _Cnt;
// volatile int _Delay;
#endif

void MPY_strn(const char *str, size_t len);
void MPY_str(const char *str);
void MPY_ch(uint8_t c);
int MPY_printf(const char *fmt, ...);

#if defined(USE_MPY_DEBUG)
#define MPY_DEBUG_TXCH   MPY_ch
#define MPY_DEBUG_TXSTR  MPY_str
#define MPY_DEBUG_TXSTRN MPY_strn
#define MPY_DEBUG_PRINTF MPY_printf
#elif defined(USE_SEGGER_RTT)
#define MPY_DEBUG_TXCH   SEGGER_RTT_ch
#define MPY_DEBUG_TXSTR  SEGGER_RTT_str
#define MPY_DEBUG_TXSTRN SEGGER_RTT_strn
#define MPY_DEBUG_PRINTF SEGGER_RTT_printf
#else
#define MPY_DEBUG_TXCH
#define MPY_DEBUG_TXSTR
#define MPY_DEBUG_TXSTRN
#define MPY_DEBUG_PRINTF
#endif

#endif /* DRIVERS_ESP_MPY_DEBUG_H_ */
