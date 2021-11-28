/*
 * Copyright (c) 2018, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RX65N_UTILS_H_
#define RX65N_UTILS_H_

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
    __asm__ __volatile__ ("setpsw i");
}

static inline uint32_t rx_disable_irq(void) {
    uint32_t state = rx_get_irq();
    __asm__ __volatile__ ("clrpsw i");
    return state;
}

#ifdef __cplusplus
}
#endif

#endif /* RX65N_UTILS_H_ */
