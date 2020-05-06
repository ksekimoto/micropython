/*
 * Copyright (c) 2020, Kentaro Sekimoto
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

#include "rza2m_utils.h"

void __WFI(void) {
    __asm__ __volatile__ ("wfi");
}


uint32_t _get_irq(void) {
    register uint32_t pri = 0;
/*
    __asm__ volatile (
        "MRC    p15,4,r1,c15,c0,0\n\t"
        "LDR    r0,[r1,#ICCPMR_OFFSET]\n\t"
        "MOV    r0,r0,LSR #3\n\t"
        "BX     lr\n\t"
        : [r0] "=r" (pri) :
    );
*/
    return pri;
}

void __enable_irq(uint32_t state) {
    __asm__ __volatile__ ("cpsie i" : : : "memory");
}

uint32_t __disable_irq(void) {
    uint32_t state = (uint32_t)_get_irq();
    __asm__ __volatile__ ("cpsid i" : : : "memory");
    return state;
}

#if RZ_TODO

__attribute__((always_inline)) void rz_enable_irq(void) {
    __asm__ __volatile__ ("cpsie i" : : : "memory");
}

__attribute__((always_inline)) void rz_disable_irq(void) {
    __asm__ __volatile__ ("cpsid i" : : : "memory");
}

__attribute__((always_inline)) uint32_t rz_get_PRIMASK(void) {
    uint32_t result;
    __asm__ __volatile__ ("MRS %0, primask" : "=r" (result));
    return (result);
}

__attribute__((always_inline) ) void rz_set_PRIMASK(uint32_t priMask) {
    __asm__ __volatile__ ("MSR primask, %0" : : "r" (priMask) : "memory");
}

#endif
