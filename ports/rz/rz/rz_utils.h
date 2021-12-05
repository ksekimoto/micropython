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

#ifndef RZ_RZ_UTILS_H_
#define RZ_RZ_UTILS_H_

#include <stdint.h>

static inline void rz_enable_irq(uint32_t state) {
    __asm__ __volatile__ ("cpsie i" : : : "memory");
}

static inline uint32_t rz_disable_irq(void) {
    uint32_t state;
    __asm__ __volatile__ ("MRS %0,APSR\n\t"
        "AND %0,%0,#0x80\n\t"
        "cpsid i" : "=r" (state) : :);
    return state;
}


#if RZ_TODO
__attribute__((always_inline)) uint32_t rz_get_PRIMASK(void);
__attribute__((always_inline)) void rz_set_PRIMASK(uint32_t priMask);
#endif

#endif /* RZ_RZ_UTILS_H_ */
