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
#include "rx_gpio.h"

// #define USE_BIT_OPERATION

void rx_gpio_config(uint8_t pin, uint8_t mode, uint8_t pull, uint8_t alt) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    switch (mode) {
        case GPIO_MODE_INPUT:
            _PMR(port) &= ~mask; /* GPIO */
            _PDR(port) &= ~mask; /* input */
            break;
        case GPIO_MODE_OUTPUT_PP:
            _PMR(port) &= ~mask; /* GPIO */
            _PCR(port) &= ~mask; /* pullup clear */
            _PDR(port) |= mask; /* output */
            break;
        case GPIO_MODE_OUTPUT_OD:
            /* N-channel open drain */
            _PMR(port) &= ~mask; /* GPIO */
            _PDR(port) |= mask; /* output */
            mask = ((uint8_t)(1 << ((pin & 3) << 1)));
            if (pin & 0x4) {
                _ODR1(port) |= mask;
            } else {
                _ODR0(port) |= mask;
            }
            break;
        case GPIO_MODE_AF_PP:
            _PMR(port) |= mask; /* AF */
            _PDR(port) |= mask; /* output */
            break;
        case GPIO_MODE_AF_OD:
            _PMR(port) |= mask; /* AF */
            _PDR(port) |= mask; /* output */
            mask = ((uint8_t)(1 << ((pin & 3) << 1)));
            if (pin & 0x4) {
                _ODR1(port) |= mask;
            } else {
                _ODR0(port) |= mask;
            }
            break;
    }
    if (mode == GPIO_MODE_INPUT) {
        switch (pull) {
            case GPIO_NOPULL:
                // assumption GPIO input mode
                _PCR(port) &= ~mask;
                break;
            case GPIO_PULLUP:
                // assumption GPIO input mode
                _PCR(port) |= mask;
                break;
        }
    }
    if (alt != 0) {
        _PMR(port) |= mask; /* AF */
        uint8_t v = (_PXXPFS(port, pin & 7) & ~0x2f) | (uint8_t)(alt & 0x2f);
        _PXXPFS(port, pin & 7) = v;
    }
}

#if defined(USE_BIT_OPERATION)
void rx_gpio_mode_output(uint8_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint32_t bit = GPIO_BIT(pin);
    bit_clr(_PPMR(port), bit);  /* gpio */
    bit_set(_PPDR(port), bit);  /* output */
    bit_clr(_PPCR(port), bit);  /* pullup clear */
}

void rx_gpio_mode_input(uint8_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint32_t bit = GPIO_BIT(pin);
    bit_clr(_PPMR(port), bit);  /* gpio */
    bit_clr(_PPDR(port), bit);  /* output */
}
#else
void rx_gpio_mode_output(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PMR(port) &= ~mask;    /* gpio */
    _PDR(port) |= mask;     /* output */
    _PCR(port) &= ~mask;    /* pullup clear */
}

void rx_gpio_mode_input(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PMR(port) &= ~mask;    /* gpio */
    _PDR(port) &= ~mask;    /* input */
}
#endif

#if defined(USE_BIT_OPERATION)
void rx_gpio_write(uint8_t pin, uint8_t state) {
    uint32_t port = GPIO_PORT(pin);
    uint32_t bit = GPIO_BIT(pin);
    bit_set(_PPDR(port), bit);   /* output */
    if (state) {
        bit_set(_PPODR(port), bit);
    } else {
        bit_clr(_PPODR(port), bit);
    }
}
#else
void rx_gpio_write(uint8_t pin, uint8_t state) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PDR(port) |= mask; /* output */
    if (state) {
        _PODR(port) |= mask;
    } else {
        _PODR(port) &= ~mask;
    }
}
#endif

void rx_gpio_toggle(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PDR(port) |= mask; /* output */
    _PODR(port) ^= mask;
}

uint8_t rx_gpio_read(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    return ((_PIDR(port) & mask) != 0) ? 1 : 0;
}

uint8_t rx_gpio_get_mode(uint8_t pin) {
    uint8_t mode = 0;
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    if ((_PDR(port) & mask) != 0) {
        // OUT
        mode = GPIO_MODE_OUTPUT_PP;
    } else {
        // IN
        mode = GPIO_MODE_INPUT;
    }
    return mode;
}

uint8_t rx_gpio_get_pull(uint8_t pin) {
    uint8_t pull = 0;
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    if ((_PCR(port) & mask) != 0) {
        pull = GPIO_PULLUP;
    } else {
        pull = GPIO_NOPULL;
    }
    return pull;
}

uint8_t rx_gpio_get_af(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    return (_PMR(port) & mask) != 0;
}
