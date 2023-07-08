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

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "iodefine.h"
#include "rx_utils.h"

extern void *HardwareVectors[];

void rx_reset(void) {
    void (*rx_start)(void);
    rx_start = (void *)((uint32_t *)HardwareVectors[0]); // PowerON_Reset
    rx_start();
}

__attribute__((weak)) int32_t
__popcountsi2(int32_t a) {
    uint32_t x = (uint32_t)a;
    x = x - ((x >> 1) & 0x55555555);
    /* Every 2 bits holds the sum of every pair of bits */
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) */
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) */
    x = (x + (x >> 16));
    /* The lower 16 bits hold two 8 bit sums (5 significant bits).*/
    /*    Upper 16 bits are garbage */
    return (x + (x >> 8)) & 0x0000003F;  /* (6 significant bits) */
}

__attribute__((weak)) int32_t
__ctzsi2(int32_t a) {
    uint32_t x = (uint32_t)a;
    int32_t t = ((x & 0x0000FFFF) == 0) << 4;  /* if (x has no small bits) t = 16 else 0 */
    x >>= t;           /* x = [0 - 0xFFFF] + higher garbage bits */
    uint32_t r = t;       /* r = [0, 16]  */
    /* return r + ctz(x) */
    t = ((x & 0x00FF) == 0) << 3;
    x >>= t;           /* x = [0 - 0xFF] + higher garbage bits */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + ctz(x) */
    t = ((x & 0x0F) == 0) << 2;
    x >>= t;           /* x = [0 - 0xF] + higher garbage bits */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + ctz(x) */
    t = ((x & 0x3) == 0) << 1;
    x >>= t;
    x &= 3;            /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */
    /* return r + ctz(x) */

/*  The branch-less return statement below is equivalent
 *  to the following switch statement:
 *     switch (x)
 *    {
 *     case 0:
 *         return r + 2;
 *     case 2:
 *         return r + 1;
 *     case 1:
 *     case 3:
 *         return r;
 *     }
 */
    return r + ((2 - (x >> 1)) & -((x & 1) == 0));
}

__attribute__((weak)) int32_t
__clzsi2(int32_t a) {
    uint32_t x = (uint32_t)a;
    int32_t t = ((x & 0xFFFF0000) == 0) << 4;  /* if (x is small) t = 16 else 0 */
    x >>= 16 - t;      /* x = [0 - 0xFFFF] */
    uint32_t r = t;       /* r = [0, 16] */
    /* return r + clz(x) */
    t = ((x & 0xFF00) == 0) << 3;
    x >>= 8 - t;       /* x = [0 - 0xFF] */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + clz(x) */
    t = ((x & 0xF0) == 0) << 2;
    x >>= 4 - t;       /* x = [0 - 0xF] */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + clz(x) */
    t = ((x & 0xC) == 0) << 1;
    x >>= 2 - t;       /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */
    /* return r + clz(x) */
/*     switch (x)
 *     {
 *     case 0:
 *         return r + 2;
 *     case 1:
 *         return r + 1;
 *     case 2:
 *     case 3:
 *         return r;
 *     }
 */
    return r + ((2 - x) & -((x & 2) == 0));
}
