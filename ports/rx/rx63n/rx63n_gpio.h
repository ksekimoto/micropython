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

#define _PXXPFS(port, bit)  ((volatile uint8_t *)(0x0008c140 + port*8 + bit))
#define _PDR(port)  (*(volatile uint8_t *)(0x0008c000 + port))
#define _PODR(port) (*(volatile uint8_t *)(0x0008c020 + port))
#define _PIDR(port) (*(volatile uint8_t *)(0x0008c040 + port))
#define _PMR(port)  (*(volatile uint8_t *)(0x0008c060 + port))
#define _ODR0(port) (*(volatile uint8_t *)(0x0008c080 + port*2))
#define _ODR1(port) (*(volatile uint8_t *)(0x0008c081 + port*2))
#define _PCR(port)  (*(volatile uint8_t *)(0x0008c0C0 + port))
#define _DSCR(port) (*(volatile uint8_t *)(0x0008c0E0 + port))
#define _MPC(pin)   (*(volatile uint8_t *)(0x0008c140 + pin))

void gpio_mode_output(uint32_t pin);
void gpio_mode_input(uint32_t pin);
void gpio_write(uint32_t pin, uint32_t state);
void gpio_toggle(uint32_t pin);
uint32_t gpio_read(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* RX63N_GPIO_H_ */
