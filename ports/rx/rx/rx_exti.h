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

#ifndef RX_RX_EXTI_H_
#define RX_RX_EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "iodefine.h"

#define IRQ0    0
#define IRQ1    1
#define IRQ2    2
#define IRQ3    3
#define IRQ4    4
#define IRQ5    5
#define IRQ6    6
#define IRQ7    7
#define IRQ8    8
#define IRQ9    9
#define IRQ10   10
#define IRQ11   11
#define IRQ12   12
#define IRQ13   13
#define IRQ14   14
#define IRQ15   15

#define NMI     16

#define RX_IR(irq_no)     (*((volatile unsigned char *)&ICU.IR[IR_ICU_IRQ0 + irq_no]))
#define RX_IER(irq_no)    (*((volatile unsigned char *)&ICU.IER[IER_ICU_IRQ0 + (irq_no / 8)]))
#define RX_IPR(irq_no)    (*((volatile unsigned char *)&ICU.IPR[IPR_ICU_IRQ0 + irq_no]))
#define RX_DTCER(irq_no)  (*((volatile unsigned char *)&ICU.DTCER[DTCE_ICU_IRQ0 + irq_no]))
#define RX_IRQCR(irq_no)  (*((volatile unsigned char *)&ICU.IRQCR[irq_no]))

#define DEFAULT_INT_PRIORITY    0x05

typedef void (*EXTI_FUNC)(void *);

uint8_t rx_exti_find_pin_irq(uint8_t idx);
void rx_exti_enable(uint32_t pin);
void rx_exti_disable(uint32_t pin);
void rx_exti_set_callback(uint32_t irq_no, EXTI_FUNC func, void *param);
void rx_exti_register(uint32_t pin, uint32_t cond, uint32_t pull);
void rx_exti_irq_clear(uint32_t irq_no);
void rx_exti_init(void);
void rx_exti_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_EXTI_H_ */
