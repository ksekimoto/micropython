//
// Copyright (c) 2017, Kentaro Sekimoto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  -Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//  -Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define NO_FLOAT

#define MORETHAN10  1
#define MAX_DIGIT   20
const char sdigit_str[] = "0123456789abcdef";
const char bdigit_str[] = "0123456789ABCDEF";

static int isdigit(register int c)
{
    return ((c >= '0') && (c <= '9'));
}

// put_s_buf
// put a string into a buffer and fill with spaces
// buf:     pointer to buffer
// s:       pointer to source string
// width:   width to be stored
// buf_size size of buffer
// minusflag    if 0, then right aligned
static unsigned int put_s_buf(char *buf, char *s, int width, unsigned int buf_size, int minusflag)
{
    unsigned int size = 0;
    if ((!minusflag) && (width > 0)) {
        // ToDo: if buf_size < width, then crash.
        width -= strlen(s); // decreased by string length
        if (width > 0) {
            memset(buf, ' ', width);
            size += width;
            buf += width;
            buf_size -= width;
        }
    }
    for (; *s != 0; s++, width--) {
        if (buf_size-- == 0)
            return -1;
        *buf++ = *s;
        size++;
    }
    for (; width > 0; width--) {
        if (buf_size-- == 0)
            return -1;
        *buf++ = ' ';
        size++;
    }
    return size;
}

// put_c_buf
// put a character into buffer and fill with spaces
// buf:     pointer to buffer
// c:       a character to be stored
// width:   width to be stored
// buf_size size of buffer
static unsigned int put_c_buf(char *buf, char c, int width, unsigned int buf_size)
{
    unsigned int size = 0;
    if (buf_size > 0) {
        if (width > 0) {
            width--;    // decrease for one character
            if (width > (int)buf_size)
                width = (int)buf_size;
            memset(buf, ' ', width);
            buf += width;
            size += width;
        }
        *buf++ = c;
        size++;
    }
    return size;
}

// put_n_buf
// put a decimal number into buffer
// buf:     pointer to buffer
// fmt:     pointer to format string
// vv:      value to be stored
// width:   width to be stored
// zeroflag flag for zero filling
// buf_size size of buffer
static unsigned int put_n_buf(char *buf, const char *fmt, unsigned long long vv, int width, unsigned int zeroflag, unsigned int buf_size)
{
    long long vvd;
    char tmp[MAX_DIGIT];
    char *p, *t, z;
    unsigned int base;
    unsigned int cap = 0;
    unsigned int size = 0;
    unsigned int negativeflag = 0;

    p = tmp;
    if (zeroflag)
        z = '0';
    else
        z = ' ';
    switch(*fmt) {
    case 'b':
        base = 2;
        break;
    case 'd':
        base = 10;
        vvd = (long long)vv;
        if (vvd < 0) {
            vv = -vvd;
            negativeflag = 1;
        }
        break;
    case 'u':
        base = 10;
        break;
    case 'o':
        base = 8;
        break;
    case 'x':
        base = 16;
        break;
    case 'D':
        base = 10;
        cap = 1;
        break;
    case 'O':
        base = 8;
        cap = 1;
        break;
    case 'X':
        base = 16;
        cap = 1;
        break;
    default:
        return size;
    }
    t = (char *)sdigit_str;
    if (cap)
        t = (char *)bdigit_str;
    do {
        width--;
        *p++ = t[vv % base];
        vv /= base;
    } while (vv != 0);
    if (!zeroflag && negativeflag) {
        width--;
        *p++ = '-';
    }
    if (zeroflag && negativeflag) {
        if (buf_size == 0)
            return -1;
        buf_size--;
        *buf++ = '-';
        size++;
        width--;
    }
    for (; width > 0; width--) {
        if (buf_size == 0)
            return -1;
        buf_size--;
        *buf++ = z;
        size++;
    }
    while (p > tmp) {
        if (buf_size == 0)
            return -1;
        buf_size--;
        *buf++ = *--p;
        size++;
    }
    return size;
}

#ifndef NO_FLOAT
// put_f_buf
// put a float number into buffer
// buf:     pointer to buffer
// fmt:     pointer to format string
// f:       float value to be stored
// width:   width for before dicimal point
// adp:     width for after dicimal point
// zeroflag flag for zero filling
// buf_size size of buffer
static int put_f_buf(char *buf, const char *fmt, double f, int width, int adp, int zeroflag, unsigned int buf_size)
{
    char tmp[MAX_DIGIT];

    long long l;
    long long base;
    char z;
    int i;
    char *p, *t;
    int size = 0;
    int negativeflag = 0;

    if (zeroflag)
        z = '0';
    else
        z = ' ';
    base = 10L;
    if (f < 0) {
        f = -f;
        negativeflag = 1;
    }
    for (i = 0; i < adp; i++)
        f *= 10.0;
    l = (long long)f;
    p = tmp;
    t = (char *)sdigit_str;
    do {
        if (adp == 0)
            *p++ = '.';
        width--;
        adp--;
        *p++ = t[l % base];
        l /= base;
    } while (l != 0);
    if (!zeroflag && negativeflag) {
        width--;
        *p++ = '-';
    }
    if (zeroflag && negativeflag) {
        if (buf_size == 0)
            return -1;
        buf_size--;
        *buf++ = '-';
        size++;
        width--;
    }
    for (; width > 0; width--) {
        if (buf_size == 0)
            return -1;
        buf_size--;
        *buf++ = z;
        size++;
    }
    while (p > tmp) {
        if (buf_size == 0)
            return -1;
        buf_size--;
        *buf++ = *--p;
        size++;
    }
    return size;
}
#endif

int vxsnprintf(char *buf, size_t buf_size, const char *fmt, va_list args)
{
    char p;
    int m;
    unsigned long v;
    unsigned long long vv;
    int size;
    int n = 0;
    int len = 0;
    int zeroflag = 0;
    int minusflag = 0;
    int widthflag = 0;
    int shortflag = 0;
    double f;

    for (; *fmt != '\0'; fmt++) {
        p = *fmt;
        if (p == '%') {
            size = 0;
            fmt++;
            p = *fmt;
            vxsnprintf_0: if (p == '%') {
                goto vxsnprintf_next;
            } else if (p == '*') {
                widthflag = 1;
                fmt++;
                p = *fmt;
                n = (unsigned long)va_arg(args, long);
            } else if (p == '-') {
                minusflag = 1;
                fmt++;
                p = *fmt;
            } else if (p == '0') {
                zeroflag = 1;
                fmt++;
                p = *fmt;
            } else if (p == '.') {
                zeroflag = 1;
                fmt++;
                p = *fmt;
                goto vxsnprintf_0;
            }
            if (isdigit(p)) {
                n = *fmt++ - '0';
#if MORETHAN10
                while (isdigit(*fmt))
                    n = n * 10 + (*fmt++ - '0');
#endif
                if (*fmt == '.') {
                    fmt++;
                    if (isdigit(*fmt)) {
                        m = *fmt++ - '0';
#if MORETHAN10
                        while (isdigit(*fmt))
                            m = m * 10 + (*fmt++ - '0');
                    }
                }
                p = *fmt;
#endif
            } else if (n == 0)
                n = -1;

            switch(p) {
            case 'I':
                if (('6' == *(fmt + 1)) && ('4' == *(fmt + 2))) {
                    vv = (unsigned long long)va_arg(args, long long);
                    fmt += 3;
                    size = put_n_buf(buf, fmt, vv, n, zeroflag, buf_size);
                }
                break;
            case 'l': {
                char p1 = *(fmt + 1);
                char p2 = *(fmt + 2);
                if ((p1 == 'd') || (p1 == 'u') || (p1 == 'x') || (p1 == 'X')) {
                    vv = (unsigned long long)va_arg(args, long);
                    fmt += 1;
                    size = put_n_buf(buf, fmt, vv, n, zeroflag, buf_size);
                } else if ((p1 == 'l') && ((p2 == 'd') || (p2 == 'u') || (p2 == 'x') || (p2 == 'X'))) {
                    vv = (unsigned long long)va_arg(args, long long);
                    fmt += 2;
                    size = put_n_buf(buf, fmt, vv, n, zeroflag, buf_size);
                }
            }
                break;
            case 's':
                v = (unsigned long)va_arg(args, long);
                size = put_s_buf(buf, (char *)v, n, buf_size, minusflag);
                break;
            case 'c':
                v = (unsigned long)va_arg(args, long);
                size = put_c_buf(buf, (char)v, n, buf_size);
                break;
            case 'd':
                size = put_n_buf(buf, fmt, (long long)va_arg(args, long), n, zeroflag, buf_size);
                break;
            case 'h':
                shortflag = 1;
                fmt++;
                p = *fmt;
                goto vxsnprintf_0;
            case 'b':
            case 'u':
            case 'o':
            case 'x':
            case 'e':
            case 'O':
            case 'X':
                if (shortflag)
                    v = (unsigned long)(va_arg(args, long) & 0xffff);
                else
                    v = (unsigned long)va_arg(args, long);
                size = put_n_buf(buf, fmt, (long long)v, n, zeroflag, buf_size);
                break;
#ifndef NO_FLOAT
            case 'f':
                f = (float)va_arg(args, double);
                size = put_f_buf(buf, fmt, f, n, m, zeroflag, buf_size);
                break;
#endif
            }
            if (size < 0)
                return -1;
            if (size) {
                buf += size;
                len += size;
                buf_size -= size;
            }
        } else {
            vxsnprintf_next: if (buf_size == 0)
                return -1;
            *buf++ = p;
            len++;
            buf_size--;
        }
    }
    *buf++ = '\0';
    return len;
}
