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
#include "interrupt_handlers.h"
#include "rx_exti.h"
#include "rx_gpio.h"

#define EXTI_DEFAULT_PRIORITY   (1)
#define IRQ_NUM (16)
#define DEFAULT_BOUNCE_PERIOD   (200)   /* 200ms */

static EXTI_FUNC exti_func[IRQ_NUM] = {0};
static void *exti_param[IRQ_NUM] = {0};
static bool bounce_flag[IRQ_NUM] = {false};
static uint32_t bounce_period[IRQ_NUM] = {0};
static uint32_t bounce_start[IRQ_NUM] = {0};

static const uint8_t pin_idx[] = {
    P00,        /* P00 0 */
    P01,        /* P01 1 */
    P02,        /* P02 2 */
    P03,        /* P03 3 */
    P05,        /* P05 5 */
    P07,        /* P07 7 */
    P10,        /* P10 8 */
    P11,        /* P11 9　*/
    P12,        /* P12 10 */
    P13,        /* P13 11 */
    P14,        /* P14 12 */
    P15,        /* P15 13 */
    P16,        /* P16 14 */
    P17,        /* P17 15 */
    P20,        /* P20 16 */
    P21,        /* P21 17 */
    P34,        /* P34 28 */
    P55,        /* P55 45 */
    P67,        /* P67 55 */
    PA1,        /* PA1 81 */
    PB0,        /* PB0 88 */
    PC0,        /* PC0 96 */
    PC1,        /* PC1 97 */
    PC6,        /* PC6 102 */
    PC7,        /* PC7 103 */
    PD0,        /* PD0 104 */
    PD1,        /* PD1 105 */
    PD2,        /* PD2 106 */
    PD3,        /* PD3 107 */
    PD4,        /* PD4 108 */
    PD5,        /* PD5 109 */
    PD6,        /* PD6 110 */
    PD7,        /* PD7 111 */
    PE5,        /* PE5 117 */
    PE6,        /* PE6 118 */
    PE7,        /* PE7 119 */
    PF5,        /* PF5 125 */
    0xff,       /* end */

};

static const uint8_t pin_irq[] = {
    8,      /* P00 0 */
    9,      /* P01 1 */
    10,     /* P02 2 */
    11,     /* P03 3 */
    13,     /* P05 5 */
    15,     /* P07 7 */
    0,      /* P10 8 */
    1,      /* P11 9　*/
    2,      /* P12 10 */
    3,      /* P13 11 */
    4,      /* P14 12 */
    5,      /* P15 13 */
    6,      /* P16 14 */
    7,      /* P17 15 */
    8,      /* P20 16 */
    9,      /* P21 17 */
    4,      /* P34 28 */
    10,     /* P55 45 */
    15,     /* P67 55 */
    11,     /* PA1 81 */
    12,     /* PB0 88 */
    14,     /* PC0 96 */
    12,     /* PC1 97 */
    13,     /* PC6 102 */
    14,     /* PC7 103 */
    0,      /* PD0 104 */
    1,      /* PD1 105 */
    2,      /* PD2 106 */
    3,      /* PD3 107 */
    4,      /* PD4 108 */
    5,      /* PD5 109 */
    6,      /* PD6 110 */
    7,      /* PD7 111 */
    5,      /* PE5 117 */
    6,      /* PE6 118 */
    7,      /* PE7 119 */
    4,      /* PF5 125 */
    0xff,   /* end */
};

uint8_t rx_exti_find_pin_irq(uint8_t idx) {
    int i = 0;
    while (pin_idx[i] != 0xff) {
        if (pin_idx[i] == idx) {
            return pin_irq[i];
        }
        i++;
    }
    return 0xff;
}

void rx_exti_irq_clear(uint32_t irq_no) {
    RX_IR(irq_no) &= ~0x01;
}

void _rx_exti_enable(uint32_t pin, uint32_t irq_no) {
    uint8_t mask2 = 1 << (irq_no & 7);
    /* IRQx input: set ISEL bit */
    _MPC(pin) |= 0x40;
    /* clear interrupt flag */
    RX_IR(irq_no) &= ~0x01;
    /* enable interrupt */
    RX_IER(irq_no) |= mask2;
}

void rx_exti_enable(uint32_t pin) {
    uint32_t irq_no = (uint32_t)rx_exti_find_pin_irq((uint8_t)pin);
    if (irq_no != 0xff) {
        _rx_exti_enable(pin, irq_no);
    }
}

void _rx_exti_disable(uint32_t pin, uint32_t irq_no) {
    uint8_t mask2 = 1 << (irq_no & 7);
    /* disable interrupt */
    RX_IER(irq_no) &= ~mask2;
    /* IRQx input: reset ISEL bit */
    _MPC(pin) &= ~0x40;
    /* clear interrupt flag */
    RX_IR(irq_no) &= ~0x01;
}

void rx_exti_disable(uint32_t pin) {
    uint32_t irq_no = (uint32_t)rx_exti_find_pin_irq((uint8_t)pin);
    if (irq_no != 0xff) {
        _rx_exti_disable(pin, irq_no);
    }
}

void rx_exti_set_callback(uint32_t irq_no, EXTI_FUNC func, void *param) {
    if (irq_no >= IRQ_NUM) {
        return;
    }
    exti_func[irq_no] = func;
    exti_param[irq_no] = param;
}

static void rx_exti_callback(uint32_t irq_no) {
    if (irq_no >= IRQ_NUM) {
        return;
    }
    if (bounce_flag[irq_no]) {
        if ((mtick() - bounce_start[irq_no]) > bounce_period[irq_no]) {
            bounce_flag[irq_no] = false;
        }
        return;
    }
    uint8_t mask2 = 1 << (irq_no & 7);
    RX_IER(irq_no) &= ~mask2;
    if (exti_func[irq_no]) {
        if (!bounce_flag[irq_no]) {
            bounce_start[irq_no] = mtick();
            bounce_flag[irq_no] = true;
            (*exti_func[irq_no])(exti_param[irq_no]);
        }
    }
    RX_IER(irq_no) |= mask2;
}

/*
 * pin: cpu pin
 * irq_no: IRQ number
 * cond: 0: low, 1: down edge, 2: up edge, 3 both edge
 */

void _rx_exti_register(uint32_t pin, uint32_t irq_no, uint32_t cond, uint32_t pull,
    uint32_t irq_priority) {
    uint8_t mask2 = 1 << (irq_no & 7);
    uint16_t mask3 = 3 << ((irq_no & 7) * 2);
    cond = (cond & 0x3) << 2;
    /* disable interrupt */
    RX_IER(irq_no) &= ~mask2;
    SYSTEM.PRCR.WORD = 0xA502;
    MPC.PWPR.BIT.B0WI = 0; /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    rx_gpio_config(pin, GPIO_MODE_INPUT, pull, 0);
    /* IRQx input: set ISEL bit */
    _MPC(pin) |= 0x40;
    // MPC.PWPR.BYTE = 0x80;     /* Disable write to PFSWE and PFS*/
    SYSTEM.PRCR.WORD = 0xA500;
    /* interrupt condition (level or edge) */
    RX_IRQCR(irq_no) = (RX_IRQCR(irq_no) & ~0x0c) | (cond << 2);
    /* clear interrupt flag */
    RX_IR(irq_no) &= ~0x01;
    /* set interrupt priority */
    RX_IPR(irq_no) = irq_priority;
    if (irq_no < 8) {
        ICU.IRQFLTC0.WORD |= mask3;
        ICU.IRQFLTE0.BYTE |= mask2;
    } else {
        ICU.IRQFLTC1.WORD |= mask3;
        ICU.IRQFLTE1.BYTE |= mask2;
    }
    /* enable interrupt */
    RX_IER(irq_no) |= mask2;
}

void rx_exti_register(uint32_t pin, uint32_t cond, uint32_t pull) {
    uint32_t irq_no = (uint32_t)rx_exti_find_pin_irq((uint8_t)pin);
    if (irq_no != 0xff) {
        _rx_exti_register(pin, irq_no, cond, pull,
            (uint32_t)EXTI_DEFAULT_PRIORITY);
    }
}

static void rx_exti_bounce_init(void) {
    for (int i = 0; i < IRQ_NUM; i++) {
        bounce_flag[i] = false;
        bounce_period[i] = DEFAULT_BOUNCE_PERIOD;
    }
}

void rx_exti_set_bounce(uint32_t irq_no, uint32_t bounce) {
    bounce_period[irq_no] = bounce;
}

void rx_exti_init(void) {
    /* set the digital filters to use PCLK/1 and disable */
    ICU.IRQFLTC0.WORD = 0U;
    ICU.IRQFLTE0.BYTE = 0U;
    ICU.IRQFLTC1.WORD = 0U;
    ICU.IRQFLTE1.BYTE = 0U;
}

void rx_exti_deinit(void) {
    uint8_t cond = 0;
    for (int irq_no = 0; irq_no < IRQ_NUM; irq_no++) {
        uint8_t mask2 = 1 << (irq_no & 7);
        RX_IER(irq_no) &= ~mask2;
        /* interrupt condition (level or edge) */
        RX_IRQCR(irq_no) = (RX_IRQCR(irq_no) & ~0x0c) | (cond << 2);
        /* clear interrupt flag */
        RX_IR(irq_no) &= ~0x01;
        /* set interrupt priority */
        RX_IPR(irq_no) = 0;
    }
    rx_exti_bounce_init();
}

void INT_Excep_ICU_IRQ0(void) {
    rx_exti_callback(0);
}

void INT_Excep_ICU_IRQ1(void) {
    rx_exti_callback(1);
}

void INT_Excep_ICU_IRQ2(void) {
    rx_exti_callback(2);
}

void INT_Excep_ICU_IRQ3(void) {
    rx_exti_callback(3);
}

void INT_Excep_ICU_IRQ4(void) {
    rx_exti_callback(4);
}

void INT_Excep_ICU_IRQ5(void) {
    rx_exti_callback(5);
}

void INT_Excep_ICU_IRQ6(void) {
    rx_exti_callback(6);
}

void INT_Excep_ICU_IRQ7(void) {
    rx_exti_callback(7);
}

void INT_Excep_ICU_IRQ8(void) {
    rx_exti_callback(8);
}

void INT_Excep_ICU_IRQ9(void) {
    rx_exti_callback(9);
}

void INT_Excep_ICU_IRQ10(void) {
    rx_exti_callback(10);
}

void INT_Excep_ICU_IRQ11(void) {
    rx_exti_callback(11);
}

void INT_Excep_ICU_IRQ12(void) {
    rx_exti_callback(12);
}

void INT_Excep_ICU_IRQ13(void) {
    rx_exti_callback(13);
}

void INT_Excep_ICU_IRQ14(void) {
    rx_exti_callback(14);
}

void INT_Excep_ICU_IRQ15(void) {
    rx_exti_callback(15);
}
