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
#include <stdarg.h>
#include "py/runtime.h"
#include "mpy_debug.h"

void MPY_strn(const char *str, size_t len) {
    mp_print_strn(MP_PYTHON_PRINTER, str, len, 0, ' ', (int)len);
}
void MPY_str(const char *str) {
    mp_printf(MP_PYTHON_PRINTER, "%s", str);
}
void MPY_ch(uint8_t c) {
    mp_printf(MP_PYTHON_PRINTER, "%c", c);
}
int MPY_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = mp_vprintf(MP_PYTHON_PRINTER, fmt, ap);
    va_end(ap);
    return ret;
}

#if defined(USE_SEGGER_RTT)

void SEGGER_RTT_strn(const char *str, size_t len);
void SEGGER_RTT_str(const char *str);
void SEGGER_RTT_ch(uint8_t c);

void SEGGER_RTT_strn(const char *str, size_t len) {
    SEGGER_RTT_Write(0, (const void *)str, (unsigned)len);
}
void SEGGER_RTT_str(const char *str) {
    SEGGER_RTT_strn(str, (size_t)strlen(str));
}
void SEGGER_RTT_ch(uint8_t c) {
    SEGGER_RTT_Write(0, (const void *)&c, (unsigned)1);
}
#endif
