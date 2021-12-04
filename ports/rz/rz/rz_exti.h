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

#ifndef RZ_RZ_EXTI_H_
#define RZ_RZ_EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// #define IRQ0    0
// #define IRQ1    1
// #define IRQ2    2
// #define IRQ3    3
// #define IRQ4    4
// #define IRQ5    5
// #define IRQ6    6
// #define IRQ7    7
// #define IRQ8    8

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
