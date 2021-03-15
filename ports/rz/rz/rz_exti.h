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

#ifndef RZ_RZ_EXTI_H_
#define RZ_RZ_EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

//#define IRQ0    0
//#define IRQ1    1
//#define IRQ2    2
//#define IRQ3    3
//#define IRQ4    4
//#define IRQ5    5
//#define IRQ6    6
//#define IRQ7    7
//#define IRQ8    8

#define IRQ_NUM 8
#define NO_IRQ  0xff
#define NO_AF  0xff
#define DEFAULT_INT_PRIORITY    0x03
#define DEFAULT_BOUNCE_PERIOD   (200)   /* 200ms */

typedef struct _exti_irq_map {
    uint16_t pinw;
    uint8_t irq_no;
    uint8_t af_no;
} exti_irq_map_t;

typedef struct _exti_irq {
    uint32_t pin;
    uint32_t int_enable;
} exti_irq_t;

#define RZ_RZ_IRQ_LOW     0
#define RZ_RZ_IRQ_FALL    1
#define RZ_RZ_IRQ_RISE    2
#define RZ_RZ_IRQ_BOTH    3

typedef void (*EXTI_ISR)(void);
typedef void (*EXTI_FUNC)(void *);

uint8_t rz_exti_find_pin_irq(uint32_t pin);
void rz_exti_enable(uint32_t pin);
void rz_exti_disable(uint32_t pin);
void rz_exti_set_callback(uint32_t irq_no, EXTI_FUNC func, void *param);
void rz_exti_register(uint32_t pin, uint32_t cond, uint32_t pull);
void rz_exti_irq_clear(uint32_t irq_no);
void rz_exti_init(void);
void rz_exti_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_EXTI_H_ */
