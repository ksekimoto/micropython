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

#ifndef RX63N_GPIO_H_
#define RX63N_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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
#define  GPIO_PULLDOWN          15

#define GPIO_PORT(pin)  ((uint8_t)(pin >> 3))
#define GPIO_MASK(pin)  ((uint8_t)(1 << (pin & 7)))

#define _PXXPFS(port, bit)  (*(volatile uint8_t *)(0x0008c140 + port*8 + (bit)))
#define _PDR(port)  (*(volatile uint8_t *)(0x0008c000 + port))
#define _PODR(port) (*(volatile uint8_t *)(0x0008c020 + port))
#define _PIDR(port) (*(volatile uint8_t *)(0x0008c040 + port))
#define _PMR(port)  (*(volatile uint8_t *)(0x0008c060 + port))
#define _ODR0(port) (*(volatile uint8_t *)(0x0008c080 + port*2))
#define _ODR1(port) (*(volatile uint8_t *)(0x0008c081 + port*2))
#define _PCR(port)  (*(volatile uint8_t *)(0x0008c0C0 + port))
#define _DSCR(port) (*(volatile uint8_t *)(0x0008c0E0 + port))
#define _MPC(pin)   (*(volatile uint8_t *)(0x0008c140 + pin))

void gpio_config(uint32_t pin, uint32_t mode, uint32_t pull, uint32_t alt);
void gpio_mode_output(uint32_t pin);
void gpio_mode_input(uint32_t pin);
void gpio_write(uint32_t pin, uint32_t state);
void gpio_toggle(uint32_t pin);
uint32_t gpio_read(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* RX63N_GPIO_H_ */
