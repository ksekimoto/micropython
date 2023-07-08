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

#ifndef RZ_RZ_GPIO_H_
#define RZ_RZ_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum CPU_PIN {
    P00 = 0x00, P01, P02, P03, P04, P05, P06, P07,
    P10 = 0x10, P11, P12, P13, P14, P15, P16, P17,
    P20 = 0x20, P21, P22, P23, P24, P25, P26, P27,
    P30 = 0x30, P31, P32, P33, P34, P35, P36, P37,
    P40 = 0x40, P41, P42, P43, P44, P45, P46, P47,
    P50 = 0x50, P51, P52, P53, P54, P55, P56, P57,
    P60 = 0x60, P61, P62, P63, P64, P65, P66, P67,
    P70 = 0x70, P71, P72, P73, P74, P75, P76, P77,
    P80 = 0x80, P81, P82, P83, P84, P85, P86, P87,
    P90 = 0x90, P91, P92, P93, P94, P95, P96, P97,
    PA0 = 0xA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
    PB0 = 0xB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7,
    PC0 = 0xC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7,
    PD0 = 0xD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7,
    PE0 = 0xE0, PE1, PE2, PE3, PE4, PE5, PE6, PE7,
    PF0 = 0xF0, PF1, PF2, PF3, PF4, PF5, PF6, PF7,
    PG0 = 0x100, PG1, PG2, PG3, PG4, PG5, PG6, PG7,
    PH0 = 0x110, PH1, PH2, PH3, PH4, PH5, PH6, PH7,
    PJ0 = 0x120, PJ1, PJ2, PJ3, PJ4, PJ5, PJ6, PJ7,
    PK0 = 0x130, PK1, PK2, PK3, PK4, PK5, PK6, PK7,
    PL0 = 0x140, PL1, PL2, PL3, PL4, PL5, PL6, PL7,
    JP00 = 0x150, JP01,
    PIN_END = 0xffff
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

#include "gpio_addrdefine.h"
#if 0
#define PORTm_base (0xFCFFE000uL)

#define PDR(g)     (volatile unsigned short *)(PORTm_base + 0x0000 + ((g) * 2))
#define PODR(g)    (volatile unsigned char *)(PORTm_base + 0x0040 + ((g) * 1))
#define PIDR(g)    (volatile unsigned char *)(PORTm_base + 0x0060 + ((g) * 1))
#define PMR(g)     (volatile unsigned char *)(PORTm_base + 0x0080 + ((g) * 1))
#define DSCR(g)    (volatile unsigned short *)(PORTm_base + 0x0140 + ((g) * 2))
#define PFS(g, n)   (volatile unsigned char *)(PORTm_base + 0x0200 + ((g) * 8) + n)
#endif

#define GPIO_PORT(pin)      (pin >> 4)
#define GPIO_MASK1(pin)     (1 << (pin & 7))
#define GPIO_MASK_DIR(pin)  (3 << ((pin & 7) << 1))
#define GPIO_DIR_HIZ(pin)   (0 << ((pin & 7) << 1))
#define GPIO_DIR_IN(pin)    (2 << ((pin & 7) << 1))
#define GPIO_DIR_OUT(pin)   (3 << ((pin & 7) << 1))
#define GPIO_BIT(pin)       (pin & 7)

#define PPDR(g)     *(volatile unsigned short *)(PORTm_base + 0x0000 + ((g) * 2))
#define PPODR(g)    *(volatile unsigned char *)(PORTm_base + 0x0040 + ((g) * 1))
#define PPIDR(g)    *(volatile unsigned char *)(PORTm_base + 0x0060 + ((g) * 1))
#define PPMR(g)     *(volatile unsigned char *)(PORTm_base + 0x0080 + ((g) * 1))
#define PDSCR(g)    *(volatile unsigned short *)(PORTm_base + 0x0140 + ((g) * 2))
#define PPFS(g, n)   *(volatile unsigned char *)(PORTm_base + 0x0200 + ((g) * 8) + n)

/*
 * asm volatile(
    AssemblerTemplate
    : OutputOperands
    : InputOperands
    : Clobbers)
 */

inline void bit_clr(uint8_t *port, uint32_t bit) {
//    __asm __volatile ( "bclr %1, [%0].b\n" : : "r" (port), "r" (bit) : );
}

inline void bit_set(uint8_t *port, uint32_t bit) {
//    __asm __volatile ( "bset %1, [%0].b\n" : : "r" (port), "r" (bit) : );
}

void rz_gpio_config(uint32_t pin, uint8_t mode, uint8_t pull, uint8_t alt);
void rz_gpio_mode_af(uint32_t pin, uint8_t af);
void rz_gpio_mode_gpio(uint32_t pin);
void rz_gpio_mode_output(uint32_t pin);
void rz_gpio_mode_input(uint32_t pin);
void rz_gpio_write(uint32_t pin, uint8_t state);
void rz_gpio_toggle(uint32_t pin);
uint8_t rz_gpio_read(uint32_t pin);
uint8_t rz_gpio_get_mode(uint32_t pin);
uint8_t rz_gpio_get_pull(uint32_t pin);
uint8_t rz_gpio_get_af(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_GPIO_H_ */
