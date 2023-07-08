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
#include "gpio_addrdefine.h"
#include "rz_gpio.h"

void rz_gpio_config(uint32_t pin, uint8_t mode, uint8_t pull, uint8_t alt) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskin = GPIO_DIR_IN(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    switch (mode) {
        case GPIO_MODE_INPUT:
            PPMR(port) &= ~mask1; /* GPIO */
            PPDR(port) = ((PPDR(port) & (~maskd))) | maskin;
            break;
        case GPIO_MODE_OUTPUT_PP:
        case GPIO_MODE_OUTPUT_OD:
            PPMR(port) &= ~mask1; /* GPIO */
            PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
            break;
        case GPIO_MODE_AF_PP:
        case GPIO_MODE_AF_OD:
            PPMR(port) |= mask1; /* AF */
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

void rz_gpio_mode_af(uint32_t pin, uint8_t af) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint32_t bit = GPIO_BIT(pin);
    GPIO.PWPR.BIT.B0WI = 0;
    GPIO.PWPR.BIT.PFSWE = 1;
    PPFS(port, bit) = (uint8_t)af;   /* af func */
    GPIO.PWPR.BIT.PFSWE = 0;
    GPIO.PWPR.BIT.B0WI = 1;
    PPMR(port) |= mask1;            /* af mode*/
}

void rz_gpio_mode_gpio(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    PPMR(port) &= ~mask1;    /* gpio */
}

void rz_gpio_mode_output(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
}

void rz_gpio_mode_input(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskin = GPIO_DIR_IN(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskin;
}

void rz_gpio_write(uint32_t pin, uint8_t state) {
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

void rz_gpio_toggle(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    uint16_t maskd = GPIO_MASK_DIR(pin);
    uint16_t maskout = GPIO_DIR_OUT(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    PPDR(port) = ((PPDR(port) & (~maskd))) | maskout;
    PPODR(port) ^= mask1;
}

uint8_t rz_gpio_read(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    PPMR(port) &= ~mask1;    /* gpio */
    return ((PPIDR(port) & mask1) != 0) ? 1 : 0;
}

uint8_t rz_gpio_get_mode(uint32_t pin) {
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

uint8_t rz_gpio_get_pull(uint32_t pin) {
    return GPIO_NOPULL;
}

uint8_t rz_gpio_get_af(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    return (PPMR(port) & mask1) != 0;
}
