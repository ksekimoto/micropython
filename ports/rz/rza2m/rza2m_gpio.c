/*
 * Copyright (c) 2020, Kentaro Sekimoto
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
//#include "interrupt_handlers.h"
#include "gpio_addrdefine.h"
#include "rza2m_gpio.h"

void _gpio_config(uint32_t pin, uint8_t mode, uint8_t pull, uint8_t alt) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskin = GPIO_DIR_IN(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    switch (mode) {
    case GPIO_MODE_INPUT:
        PPMR(port) &= ~mask1;    /* GPIO */
        PPDR(port) = ((PPDR(port) & (~maskd))) | maskin;
        break;
    case GPIO_MODE_OUTPUT_PP:
    case GPIO_MODE_OUTPUT_OD:
        PPMR(port) &= ~mask1;    /* GPIO */
        PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
        break;
    case GPIO_MODE_AF_PP:
    case GPIO_MODE_AF_OD:
        PPMR(port) |= mask1;     /* AF */
        PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
        break;
    }
    if (mode == GPIO_MODE_INPUT) {
        switch (pull) {
        case GPIO_NOPULL:
            break;
        case GPIO_PULLUP:
            break;
        }
    }
}

void _gpio_mode_af(uint32_t pin, uint8_t af) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint32_t bit = GPIO_BIT(pin);
    GPIO.PWPR.BIT.B0WI  = 0;
    GPIO.PWPR.BIT.PFSWE = 1;
    PPFS(port,bit) = (uint8_t)af;   /* af func */
    GPIO.PWPR.BIT.PFSWE = 0;
    GPIO.PWPR.BIT.B0WI  = 1;
    PPMR(port) |= mask1;            /* af mode*/
}

void _gpio_mode_gpio(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    PPMR(port) &= ~mask1;    /* gpio */
}

void _gpio_mode_output(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
}

void _gpio_mode_input(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskin = GPIO_DIR_IN(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskin;
}

void _gpio_write(uint32_t pin, uint8_t state) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
    if (state) {
        PPODR(port) |= mask1;
    } else {
        PPODR(port) &= ~mask1;
    }
}

void _gpio_toggle(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
    PPODR(port) ^= mask1;
}

uint8_t _gpio_read(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    return ((PPIDR(port) & mask1) != 0) ? 1 : 0;
}

uint8_t _gpio_get_mode(uint32_t pin) {
    uint8_t mode = 0;
    uint32_t port = GPIO_PORT(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskin = GPIO_DIR_IN(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    if ((PPDR(port) & maskd) == maskout) {
        // OUT
        mode = GPIO_MODE_OUTPUT_PP;
    } else if ((PPDR(port) & maskd) == maskin) {
        // IN
        mode = GPIO_MODE_INPUT;
    } else {
        mode = GPIO_NOPULL; // ToDo
    }
    return mode;
}

uint8_t _gpio_get_pull(uint32_t pin) {
    return GPIO_NOPULL;
}

uint8_t _gpio_get_af(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    return ((PPMR(port) & mask1) != 0);
}
