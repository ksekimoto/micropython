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

#ifndef RX_RX_GPIO_H_
#define RX_RX_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct rx_af_pin {
    uint8_t af;
    uint8_t ch;
    uint32_t pin;
} rx_af_pin_t;

enum CPU_PIN {
    P00 = 0x00, P01, P02, P03, P04, P05, P06, P07,
    P10 = 0x08, P11, P12, P13, P14, P15, P16, P17,
    P20 = 0x10, P21, P22, P23, P24, P25, P26, P27,
    P30 = 0x18, P31, P32, P33, P34, P35, P36, P37,
    P40 = 0x20, P41, P42, P43, P44, P45, P46, P47,
    P50 = 0x28, P51, P52, P53, P54, P55, P56, P57,
    P60 = 0x30, P61, P62, P63, P64, P65, P66, P67,
    P70 = 0x38, P71, P72, P73, P74, P75, P76, P77,
    P80 = 0x40, P81, P82, P83, P84, P85, P86, P87,
    P90 = 0x48, P91, P92, P93, P94, P95, P96, P97,
    PA0 = 0x50, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
    PB0 = 0x58, PB1, PB2, PB3, PB4, PB5, PB6, PB7,
    PC0 = 0x60, PC1, PC2, PC3, PC4, PC5, PC6, PC7,
    PD0 = 0x68, PD1, PD2, PD3, PD4, PD5, PD6, PD7,
    PE0 = 0x70, PE1, PE2, PE3, PE4, PE5, PE6, PE7,
    PF0 = 0x78, PF1, PF2, PF3, PF4, PF5, PF6, PF7,
    PG0 = 0x80, PG1, PG2, PG3, PG4, PG5, PG6, PG7,
    PJ0 = 0x90, PJ1, PJ2, PJ3, PJ4, PJ5, PJ6, PJ7,
    PIN_END = 0xff,
};

enum AF_INDEX {
    AF_GPIO = 0,
    AF_MTU1 = 1,
    AF_MTU2 = 2,
    AF_TIO = 3,
    AF_TCLK = 4,
    AF_TM = 5,
    AF_PO1 = 6,
    AF_PO2 = 7,
    AF_MTU3 = 8,
    AF_ADT = 9,
    AF_SCI1 = 10,
    AF_SCI2 = 11,
    AF_SPI = 13,
    AF_RIIC = 15,
    AF_USB_ET = 17,
    AF_USB_RMII = 18,
    AF_USB1 = 19,
    AF_ED = 24,
    AF_USB2 = 35,
    AF_END = 0xff,
};

#define  GPIO_MODE_INPUT        1
#define  GPIO_MODE_OUTPUT_PP    2
#define  GPIO_MODE_OUTPUT_OD    3
#define  GPIO_MODE_AF_PP        4
#define  GPIO_MODE_AF_OD        5   /* N-channel open drain */
#define  GPIO_MODE_ANALOG       6
#define  GPIO_MODE_IT_RISING    7
#define  GPIO_MODE_IT_FALLING   8
#define  GPIO_MODE_IT_RISING_FALLING 9
#define  GPIO_MODE_EVT_RISING   10
#define  GPIO_MODE_EVT_FALLING  11
#define  GPIO_MODE_EVT_RISING_FALLING   12
#define  GPIO_NOPULL            13
#define  GPIO_PULLUP            14

#define IS_GPIO_MODE(MODE) (((MODE) == GPIO_MODE_INPUT) || \
    ((MODE) == GPIO_MODE_OUTPUT_PP) || \
    ((MODE) == GPIO_MODE_OUTPUT_OD) || \
    ((MODE) == GPIO_MODE_AF_PP) || \
    ((MODE) == GPIO_MODE_AF_OD) || \
    ((MODE) == GPIO_MODE_IT_RISING) || \
    ((MODE) == GPIO_MODE_IT_FALLING) || \
    ((MODE) == GPIO_MODE_IT_RISING_FALLING) || \
    ((MODE) == GPIO_MODE_EVT_RISING) || \
    ((MODE) == GPIO_MODE_EVT_FALLING) || \
    ((MODE) == GPIO_MODE_EVT_RISING_FALLING) || \
    ((MODE) == GPIO_MODE_ANALOG))

#define IS_GPIO_PULL(PULL) (((PULL) == GPIO_NOPULL) || ((PULL) == GPIO_PULLUP))

#define IS_GPIO_AF(AF)   ((AF) <= (uint8_t)0x0F)

#define GPIO_PORT(pin)  (pin >> 3)
#define GPIO_MASK(pin)  (1 << (pin & 7))
#define GPIO_BIT(pin)   (pin & 7)

#define _PXXPFS(port, bit)  (*(volatile uint8_t *)(0x0008c140 + ((uint32_t)port) * 8 + ((uint32_t)bit)))
#define _PDR(port)  (*(volatile uint8_t *)(0x0008c000 + ((uint32_t)port)))
#define _PODR(port) (*(volatile uint8_t *)(0x0008c020 + ((uint32_t)port)))
#define _PIDR(port) (*(volatile uint8_t *)(0x0008c040 + ((uint32_t)port)))
#define _PMR(port)  (*(volatile uint8_t *)(0x0008c060 + ((uint32_t)port)))
#define _ODR0(port) (*(volatile uint8_t *)(0x0008c080 + ((uint32_t)port) * 2))
#define _ODR1(port) (*(volatile uint8_t *)(0x0008c081 + ((uint32_t)port) * 2))
#define _PCR(port)  (*(volatile uint8_t *)(0x0008c0C0 + ((uint32_t)port)))
#define _DSCR(port) (*(volatile uint8_t *)(0x0008c0E0 + ((uint32_t)port)))
#define _MPC(pin)   (*(volatile uint8_t *)(0x0008c140 + ((uint32_t)pin)))

#define _PPXXPFS(port, bit)  ((volatile uint8_t *)(0x0008c140 + port * 8 + ((uint32_t)bit))))
#define _PPDR(port)  ((volatile uint8_t *)(0x0008c000 + ((uint32_t)port)))
#define _PPODR(port) ((volatile uint8_t *)(0x0008c020 + ((uint32_t)port)))
#define _PPIDR(port) ((volatile uint8_t *)(0x0008c040 + ((uint32_t)port)))
#define _PPMR(port)  ((volatile uint8_t *)(0x0008c060 + ((uint32_t)port)))
#define _PODR0(port) ((volatile uint8_t *)(0x0008c080 + ((uint32_t)port) * 2))
#define _PODR1(port) ((volatile uint8_t *)(0x0008c081 + ((uint32_t)port) * 2))
#define _PPCR(port)  ((volatile uint8_t *)(0x0008c0C0 + ((uint32_t)port)))
#define _PDSCR(port) ((volatile uint8_t *)(0x0008c0E0 + ((uint32_t)port)))
#define _PMPC(pin)   ((volatile uint8_t *)(0x0008c140 + ((uint32_t)pin)))

/*
 * asm volatile(
    AssemblerTemplate
    : OutputOperands
    : InputOperands
    : Clobbers)
 */

inline void bit_clr(uint8_t *port, uint32_t bit) {
    __asm __volatile("bclr %1, [%0].b\n" : : "r" (port), "r" (bit) :);
}

inline void bit_set(uint8_t *port, uint32_t bit) {
    __asm __volatile("bset %1, [%0].b\n" : : "r" (port), "r" (bit) :);
}

void rx_gpio_config(uint8_t pin, uint8_t mode, uint8_t pull, uint8_t alt);
void rx_gpio_mode_output(uint8_t pin);
void rx_gpio_mode_input(uint8_t pin);
void rx_gpio_write(uint8_t pin, uint8_t state);
void rx_gpio_toggle(uint8_t pin);
uint8_t rx_gpio_read(uint8_t pin);
uint8_t rx_gpio_get_mode(uint8_t pin);
uint8_t rx_gpio_get_pull(uint8_t pin);
uint8_t rx_gpio_get_af(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_GPIO_H_ */
