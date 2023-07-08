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

#ifndef RX_RX_UTILS_H_
#define RX_RX_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

static inline uint32_t rx_get_irq(void) {
    register uint32_t temp;
    register uint32_t pri;
    __asm__ volatile (
        "mvfc psw, %[r14]\n\t"
        "revl %[r14], %[r1]\n\t"
        "and #0x0f, %[r1]\n\t"
        : [r14] "=r" (temp), [r1] "=r" (pri)
        );
    return pri;
}

static inline uint32_t rx_get_int(void) {
    uint32_t ipl;
    __asm__ __volatile__ ("mvfc psw,%0" : "=r" (ipl) :);
    return (ipl & 0x00010000) >> 16;
}

static inline void rx_enable_irq(uint32_t state) {
    (void)state;
    __asm__ __volatile__ ("setpsw i");
}

static inline uint32_t rx_disable_irq(void) {
    uint32_t state = rx_get_irq();
    __asm__ __volatile__ ("clrpsw i");
    return state;
}

void rx_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_UTILS_H_ */
