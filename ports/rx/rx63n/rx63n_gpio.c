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

#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx63n_gpio.h"

void gpio_mode_output(uint32_t pin)
{
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PMR(port) &= ~mask;
    _PDR(port) |= mask;
}

void gpio_mode_input(uint32_t pin)
{
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PMR(port) &= ~mask;
    _PDR(port) &= ~mask;
}

void gpio_write(uint32_t pin, uint32_t state)
{
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    _PDR(port) |= mask;
    if (state)
        _PODR(port) |= mask;
    else
        _PODR(port) &= ~mask;
}

uint32_t gpio_read(uint32_t pin)
{
    uint32_t port = GPIO_PORT(pin);
    uint32_t mask = GPIO_MASK(pin);
    return ((_PIDR(port) & mask) != 0)? 1 : 0;
}
