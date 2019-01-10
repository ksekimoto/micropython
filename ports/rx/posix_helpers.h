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

#ifndef PORTS_RX_POSIX_HELPERS_H_
#define PORTS_RX_POSIX_HELPERS_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

uint32_t ntohl(uint32_t netlong);
uint32_t htonl(uint32_t netlong);

#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval {
  time_t      tv_sec;
  suseconds_t tv_usec;
};
#endif

time_t time(time_t *t);
time_t mktime(void *tm);
struct tm *gmtime(const time_t *timer, struct tm *tmbuf);
int gettimeofday(struct timeval *tv , void *tz);
sighandler_t signal (int sig, sighandler_t handler);

int atoi(const char *s);
char *itoa(int num, char *str, int base);

#endif /* PORTS_RX_POSIX_HELPERS_H_ */
