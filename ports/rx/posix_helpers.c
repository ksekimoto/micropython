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

#include "py/mphal.h"
#include "py/gc.h"

#if MICROPY_PY_LWIP
#include "lwip_utils.h"
#endif

#include "rng.h"
#include "posix_helpers.h"

//#define DEBUG_TIME
//#define DEBUG_GMTIM
//#define DEBUG_MKTIME
//#define DEBUG_GETTIMEOFDAY

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
#if defined(DEBUG_TIME)
    debug_printf("time\r\n");
#endif
//#if MICROPY_PY_LWIP
//    return get_sntp_time();
//#else
    return mp_hal_ticks_ms() / 1000;
//#endif
}

time_t mktime(void *tm) {
    // ToDo: implementation
#if defined(DEBUG_MKTIME)
    debug_printf("mktime\r\n");
#endif
    return 0;
}

struct tm *gmtime(const time_t *timer, struct tm *tmbuf) {
    // ToDo: implementation
#if defined(DEBUG_GMTIME)
    debug_printf("gmtime\r\n");
#endif
    return tmbuf;
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
#if defined(DEBUG_GETTIMEOFDAY)
    debug_printf("gettimeofday\r\n");
#endif
    return mp_hal_ticks_ms();
}

#if !MICROPY_SSL_AXTLS
sighandler_t signal (int sig, sighandler_t handler) {
    // ToDo: implementation
    return (sighandler_t)handler;
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


