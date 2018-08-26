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

#include <stdint.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx63n_exti.h"
#include "rx63n_gpio.h"


static EXTI_FUNC exti_func[16] = {0};
static void *exti_param[16] = {0};
/*
 * pin: cpu pin
 * irq_no: IRQ number
 * cond: 0: low, 1: down edge, 2: up edge, 3 both edge
 */

void exti_enable(uint32_t pin, uint32_t irq_no, uint32_t cond, uint32_t irq_priority) {
    unsigned char mask = 1 << (pin & 7);
    cond = (cond & 0x3) << 2;
    /* disable interrupt */
    RX_IER(irq_no) &= ~mask;
    gpio_config(pin, GPIO_MODE_INPUT, GPIO_PULLUP, 0);
    /* IRQx input: set ISEL bit */
    _MPC(pin) |= 0x40;
    /* interrupt condition (level or edge) */
    RX_IRQCR(irq_no) |= cond;
    /* clear interrupt flag */
    RX_IR(irq_no) &= ~mask;
    /* set interrupt priority */
    RX_IPR(irq_no) = irq_priority;
    /* enable interrupt */
    RX_IER(irq_no) |= mask;
}

void exti_disable(uint32_t pin, uint32_t irq_no) {
    unsigned char mask = 1 << (pin & 7);
    /* disable interrupt */
    RX_IER(irq_no) &= ~mask;
    //gpio_config(pin, GPIO_MODE_INPUT, GPIO_PULLUP, 0);
    /* IRQx input: reset ISEL bit */
    _MPC(pin) &= ~0x40;
    /* interrupt condition (level or edge) */
    //_IRQCR(irq_no) |= mask;
    /* clear interrupt flag */
    RX_IR(irq_no) &= ~mask;
    /* set interrupt priority */
    //_IPR(irq_no) = irq_priority;
}

void exti_set_callback(uint32_t irq_no, void (*func)(void *)) {
    if (irq_no >= 16)
        return;
    exti_func[irq_no] = func;
}

void exti_callback(uint32_t irq_no) {
    if (irq_no >= 16)
        return;
    if (exti_func[irq_no]) {
        (*exti_func[irq_no])(exti_param[irq_no]);
    }
}

void INT_Excep_ICU_IRQ0(void) {
    exti_callback(0);
}

void INT_Excep_ICU_IRQ1(void) {
    exti_callback(1);
}

void INT_Excep_ICU_IRQ2(void) {
    exti_callback(2);
}

void INT_Excep_ICU_IRQ3(void) {
    exti_callback(3);
}

void INT_Excep_ICU_IRQ4(void) {
    exti_callback(4);
}

void INT_Excep_ICU_IRQ5(void) {
    exti_callback(5);
}

void INT_Excep_ICU_IRQ6(void) {
    exti_callback(6);
}

void INT_Excep_ICU_IRQ7(void) {
    exti_callback(7);
}

void INT_Excep_ICU_IRQ8(void) {
    exti_callback(8);
}

void INT_Excep_ICU_IRQ9(void) {
    exti_callback(9);
}

void INT_Excep_ICU_IRQ10(void) {
    exti_callback(10);
}

void INT_Excep_ICU_IRQ11(void) {
    exti_callback(11);
}

void INT_Excep_ICU_IRQ12(void) {
    exti_callback(12);
}

void INT_Excep_ICU_IRQ13(void) {
    exti_callback(13);
}

void INT_Excep_ICU_IRQ14(void) {
    exti_callback(14);
}

void INT_Excep_ICU_IRQ15(void) {
    exti_callback(15);
}

