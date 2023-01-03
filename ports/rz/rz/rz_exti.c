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
#include "common.h"
#include "iodefine.h"
#include "rz_exti.h"
#include "rz_gpio.h"
#include "rz_timer.h"
#include "rz_exti.h"

unsigned long mtick(void);

static void exti_irq0(void);
static void exti_irq1(void);
static void exti_irq2(void);
static void exti_irq3(void);
static void exti_irq4(void);
static void exti_irq5(void);
static void exti_irq6(void);
static void exti_irq7(void);

static const exti_irq_map_t EXTI_IRQ_MAP[] = {
    { P62, 0, 6 },
    { PL4, 0, 5 },
    { PD0, 0, 2 },
    { PJ6, 0, 5 },
    { PJ1, 0, 6 },
    { P54, 0, 2 },
    { P10, 0, 3 },
    { P40, 0, 6 },
    { PC5, 0, 6 },
    { PF4, 1, 6 },
    { PE1, 1, 6 },
    { PD1, 1, 2 },
    { PF7, 1, 5 },
    { P11, 1, 3 },
    { PC4, 1, 6 },
    { P55, 1, 2 },
    { P41, 1, 6 },
    { P82, 2, 5 },
    { PH1, 2, 6 },
    { PD2, 2, 2 },
    { PH4, 2, 6 },
    { P12, 2, 3 },
    { P56, 2, 2 },
    { PC0, 2, 5 },
    { P42, 2, 6 },
    { PH0, 3, 6 },
    { PD3, 3, 2 },
    { P81, 3, 5 },
    { PH3, 3, 6 },
    { P43, 3, 6 },
    { P57, 3, 2 },
    { P30, 3, 5 },
    { P13, 3, 3 },
    { PL0, 4, 5 },
    { PF1, 4, 6 },
    { PD4, 4, 2 },
    { PG2, 4, 6 },
    { PH6, 4, 5 },
    { PJ5, 4, 6 },
    { P14, 4, 3 },
    { P50, 4, 2 },
    { PL1, 5, 5 },
    { PA5, 5, 6 },
    { PK2, 5, 6 },
    { PD5, 5, 2 },
    { PH5, 5, 5 },
    { PG6, 5, 6 },
    { P20, 5, 3 },
    { P51, 5, 2 },
    { PL2, 6, 5 },
    { PA1, 6, 6 },
    { PD6, 6, 2 },
    { PK4, 6, 6 },
    { P31, 6, 6 },
    { P52, 6, 2 },
    { PC7, 6, 6 },
    { P21, 6, 3 },
    { PL3, 7, 5 },
    { PD7, 7, 2 },
    { P33, 7, 6 },
    { P22, 7, 3 },
    { PC6, 7, 6 },
    { P53, 7, 2 },
};
#define EXTI_IRQ_MAP_SIZE   (sizeof(EXTI_IRQ_MAP) / sizeof(exti_irq_map_t))

static EXTI_FUNC exti_func[IRQ_NUM] = {0};
static void *exti_param[IRQ_NUM] = {0};
static bool bounce_flag[IRQ_NUM] = {false};
static uint32_t bounce_period[IRQ_NUM] = {0};
static uint32_t bounce_start[IRQ_NUM] = {0};
static exti_irq_t exit_irq_obj[IRQ_NUM] = {0};

static const EXTI_ISR irq_tbl[IRQ_NUM] = {
    &exti_irq0,
    &exti_irq1,
    &exti_irq2,
    &exti_irq3,
    &exti_irq4,
    &exti_irq5,
    &exti_irq6,
    &exti_irq7,
};

uint8_t rz_exti_find_pin_irq(uint32_t pin) {
    int i;
    for (i = 0; i < EXTI_IRQ_MAP_SIZE; i++) {
        if (EXTI_IRQ_MAP[i].pinw == (uint16_t)pin) {
            return EXTI_IRQ_MAP[i].irq_no;
        }
    }
    return NO_IRQ;
}

void rz_exti_irq_clear(uint32_t irq_no) {
    INTC.IRQRR.WORD &= ~(1 << irq_no);
}

void rz_exti_enable(uint32_t pin) {
    uint32_t irq_no = (uint32_t)rz_exti_find_pin_irq(pin);
    if (irq_no != NO_IRQ) {
        GIC_EnableIRQ((IRQn_Type)(IRQ0_IRQn + irq_no));
        exit_irq_obj[irq_no].int_enable = 1;
    }
}

void rz_exti_disable(uint32_t pin) {
    uint32_t irq_no = (uint32_t)rz_exti_find_pin_irq(pin);
    if (irq_no != NO_IRQ) {
        GIC_DisableIRQ((IRQn_Type)(IRQ0_IRQn + irq_no));
        exit_irq_obj[irq_no].int_enable = 0;
    }
}

uint8_t rz_exti_find_pin_af_no(uint32_t pin) {
    int i;
    for (i = 0; i < EXTI_IRQ_MAP_SIZE; i++) {
        if (EXTI_IRQ_MAP[i].pinw == (uint16_t)pin) {
            return EXTI_IRQ_MAP[i].af_no;
        }
    }
    return NO_AF;
}

void rz_exti_set_callback(uint32_t irq_no, EXTI_FUNC func, void *param) {
    if (irq_no >= IRQ_NUM) {
        return;
    }
    exti_func[irq_no] = func;
    exti_param[irq_no] = param;
}

void rz_exti_irq_handler(uint32_t irq_no) {
    uint16_t irqrr;
//    uint32_t pin;
    irqrr = INTC.IRQRR.WORD;
    if (irqrr & (1 << irq_no)) {
        if (bounce_flag[irq_no]) {
            if (((uint32_t)mtick() - bounce_start[irq_no]) > bounce_period[irq_no]) {
                bounce_flag[irq_no] = false;
            }
        } else {
            //           pin = exit_irq_obj[irq_no].pin;
            // GIC_DisableIRQ((IRQn_Type)(IRQ0_IRQn + irq_no));
            if (exti_func[irq_no]) {
                if (!bounce_flag[irq_no]) {
                    bounce_start[irq_no] = mtick();
                    bounce_flag[irq_no] = true;
                    (*exti_func[irq_no])(exti_param[irq_no]);
                }
            }
            // GIC_EnableIRQ((IRQn_Type)(IRQ0_IRQn + irq_no));
        }
        INTC.IRQRR.WORD &= ~(1 << irq_no);
    }
}

/*
 * pin: cpu pin
 * irq_no: IRQ number
 * cond: 0: low, 1: down edge, 2: up edge, 3 both edge
 */

void _rz_exti_register(uint32_t pin, uint32_t irq_no, uint32_t cond, uint32_t pull, uint32_t irq_priority) {
    uint8_t af_no;
    if (irq_no != (uint32_t)rz_exti_find_pin_irq(pin)) {
        return;
    }
    exit_irq_obj[irq_no].pin = pin;
    af_no = rz_exti_find_pin_af_no(pin);
    rz_gpio_mode_af(pin, af_no + 0x40);
    INTC.ICR1.WORD = (uint16_t)cond;
    InterruptHandlerRegister((IRQn_Type)(IRQ0_IRQn + irq_no), (void (*)(uint32_t))irq_tbl[irq_no]);
    GIC_SetPriority((IRQn_Type)(IRQ0_IRQn + irq_no), irq_priority);
    GIC_SetConfiguration((IRQn_Type)(IRQ0_IRQn + irq_no), 1);
    rz_exti_enable(pin);
}

void rz_exti_register(uint32_t pin, uint32_t cond, uint32_t pull) {
    uint32_t irq_no = (uint32_t)rz_exti_find_pin_irq(pin);
    if (irq_no != NO_IRQ) {
        _rz_exti_register(pin, irq_no, cond, pull, (uint32_t)DEFAULT_INT_PRIORITY);
    }
}

static void rz_exti_bounce_init(void) {
    for (int i = 0; i < IRQ_NUM; i++) {
        bounce_flag[i] = false;
        bounce_period[i] = DEFAULT_BOUNCE_PERIOD;
    }
}

void rz_exti_set_bounce(uint32_t irq_no, uint32_t bounce) {
    bounce_period[irq_no] = bounce;
}

void rz_exti_init(void) {
    int i;
    for (i = 0; i < IRQ_NUM; i++) {
        exit_irq_obj[i].int_enable = 0;
        exit_irq_obj[i].pin = PIN_END;
    }
}

void rz_exti_deinit(void) {
    int i;
    for (i = 0; i < IRQ_NUM; i++) {
        GIC_DisableIRQ((IRQn_Type)(IRQ0_IRQn + i));
        if (exit_irq_obj[i].pin != PIN_END) {
            rz_gpio_mode_af(exit_irq_obj[i].pin, 0);
        }
        exit_irq_obj[i].int_enable = 0;
        exit_irq_obj[i].pin = PIN_END;
    }
    rz_exti_bounce_init();
}

static void exti_irq0(void) {
    rz_exti_irq_handler(0);
}

static void exti_irq1(void) {
    rz_exti_irq_handler(1);
}

static void exti_irq2(void) {
    rz_exti_irq_handler(2);
}

static void exti_irq3(void) {
    rz_exti_irq_handler(3);
}

static void exti_irq4(void) {
    rz_exti_irq_handler(4);
}

static void exti_irq5(void) {
    rz_exti_irq_handler(5);
}

static void exti_irq6(void) {
    rz_exti_irq_handler(6);
}

static void exti_irq7(void) {
    rz_exti_irq_handler(7);
}
