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

#ifndef RX63N_EXTI_H_
#define RX63N_EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

void exti_enable(uint32_t pin, uint32_t irq_no, uint32_t cond, uint32_t irq_priority);
void exti_disable(uint32_t pin, uint32_t irq_no);
void exti_set_callback(uint32_t irq_no, void (*func)(void *));

#ifdef __cplusplus
}
#endif

#endif /* RX63N_EXTI_H_ */
