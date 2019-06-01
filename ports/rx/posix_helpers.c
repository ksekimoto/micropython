/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Paul Sokolovsky
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
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "py/mphal.h"
#include "py/gc.h"

//#if MICROPY_PY_LWIP
//#include "sntp_client.h"
//#endif

#include "rng.h"
#include "posix_helpers.h"

//#define DEBUG_TIME
//#define DEBUG_SIGNAL

// Functions for external libs like axTLS, BerkeleyDB, etc.

#if 0
void *malloc(size_t size) {
    void *p = gc_alloc(size, false);
    if (p == NULL) {
        // POSIX requires ENOMEM to be set in case of error
        errno = ENOMEM;
    }
    return p;
}
void free(void *ptr) {
    gc_free(ptr);
}

void *realloc(void *ptr, size_t size) {
    void *p = gc_realloc(ptr, size, true);
    if (p == NULL) {
        // POSIX requires ENOMEM to be set in case of error
        errno = ENOMEM;
    }
    return p;
}
#endif

void *calloc(size_t nmemb, size_t size) {
    void *ptr;
    if (nmemb == 0 || size == 0) {
      nmemb = size = 1;
    }
    ptr = malloc (nmemb * size);
    if (ptr) {
        memset(ptr, nmemb * size, 0);
    }
    return ptr;
}

#undef htonl
#undef ntohl
uint32_t ntohl(uint32_t netlong) {
    return MP_BE32TOH(netlong);
}
uint32_t htonl(uint32_t netlong) {
    return MP_HTOBE32(netlong);
}

time_t time(time_t *t) {
    // ToDo: implementation
//#if MICROPY_PY_LWIP
//    return get_sntp_time();
//#else
//    return mp_hal_ticks_ms() / 1000;
//#endif
    rtc_t rtc;
    struct tm tmbuf;
    time_t  ret;
    rx_rtc_get_time(&rtc);
    tmbuf.tm_hour = (int)rtc.hour;
    tmbuf.tm_mday = (int)rtc.date;
    tmbuf.tm_min = (int)rtc.minute;
    tmbuf.tm_mon = (int)rtc.month;
    tmbuf.tm_sec = (int)rtc.second;
    tmbuf.tm_wday = (int)rtc.weekday;
    tmbuf.tm_year = (int)rtc.year;
    tmbuf.tm_yday = 0;
    tmbuf.tm_isdst = 0;
    tmbuf.tm_year -= 1900;
    ret = (time_t)mktime(&tmbuf);
    if (t) {
        *t = ret;
    }
#if defined(DEBUG_TIME)
    debug_printf("time:%d\r\n", ret);
#endif
    return ret;
}

void set_time(time_t *t) {
    rtc_t rtc;
    struct tm *tmbuf;
    tmbuf = gmtime((const time_t *)t);
    rtc.hour = tmbuf->tm_hour;
    rtc.date = tmbuf->tm_mday;
    rtc.minute = tmbuf->tm_min;
    rtc.month = tmbuf->tm_mon + 1;
    rtc.second = tmbuf->tm_sec;
    rtc.weekday = tmbuf->tm_wday;
    rtc.year = tmbuf->tm_year;
    rx_rtc_set_time(&rtc);
}

#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval {
  time_t      tv_sec;
  suseconds_t tv_usec;
};
#endif

int gettimeofday(struct timeval *tv , void *tz) {
    // ToDo: implementation
    //return mp_hal_ticks_ms();
    time_t t;
    t = time(NULL);
    tv->tv_sec = t;
    tv->tv_usec = 0;
#if defined(DEBUG_GETTIMEOFDAY)
    debug_printf("gettimeofday:%d\r\n", t);
#endif
    return (int)0;
}

#if !MICROPY_SSL_AXTLS
_sig_func_ptr signal (int sig, _sig_func_ptr handler) {
    // ToDo: implementation
#if defined(DEBUG_SIGNAL)
    debug_printf("signal\r\n");
#endif
    return (_sig_func_ptr)handler;
}
#endif

int atoi(const char *s) {
    int result = 0, sign = 1;
    if (*s == -1) {
        sign = -1;
        s++;
    }
    while (*s >= '0' && *s <= '9')
        result = result * 10 + (*(s++) - '0');
    return result * sign;
}

inline void swap(char *x, char *y) {
    char t = *x;
    *x = *y;
    *y = t;
}

void reverse(char *str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        swap(str + start, str + end);
        start++;
        end--;
    }
}

// ToDo: need validation
char *itoa(int num, char *str, int base) {
    int i = 0;
    bool minus = false;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    if (num < 0 && base == 10) {
        minus = true;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    if (minus)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str, i);
    return str;
}

int rand(void) {
    return (int)rng_get();
}

int rand_r(int seed) {
    return (int)rng_get();
}

void __attribute__((noreturn)) abort(void) {
    __asm__ __volatile__ ("nop");
    while (1) {
        ;
    }
}

char *strncpy(char *dst, const char *src, size_t len) {
    char *q = (char *)dst;
    const char *p = (const char *)src;
    char ch;
    while (len--) {
        *q++ = ch = *p++;
        if (!ch)
            break;
    }
    return dst;
}

