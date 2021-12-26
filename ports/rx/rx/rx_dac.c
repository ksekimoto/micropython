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

#include <stdbool.h>
#include "iodefine.h"
#include "rx_gpio.h"

static uint8_t rx_dac_pins[] = {
    P03,
    P05,
};
#define RX_DAC_SIZE (sizeof(rx_dac_pins) / sizeof(uint8_t))

uint8_t rx_dac_get_channel(uint8_t pin_idx) {
    uint8_t channel = 0xff;
    for (int i = 0; i < RX_DAC_SIZE; i++) {
        if (rx_dac_pins[i] == pin_idx) {
            channel = i;
            break;
        }
    }
    return channel;
}

void rx_dac_write(uint8_t pin_idx, uint16_t value) {
    uint8_t channel;
    if ((channel = rx_dac_get_channel(pin_idx)) == 0xff) {
        return;
    }
    if (channel == 0) {
        DA.DADR0 = (uint16_t)value;
    }
    if (channel == 1) {
        DA.DADR1 = (uint16_t)value;
    }
}

static void rx_dac_enable(uint8_t pin_idx) {
    uint8_t port = GPIO_PORT(pin_idx);
    uint8_t mask = GPIO_MASK(pin_idx);
    MPC.PWPR.BIT.B0WI = 0;  /* Enable write to PFSWE */
    MPC.PWPR.BIT.PFSWE = 1; /* Enable write to PFS */
    _PMR(port) &= ~mask;
    _PDR(port) |= mask;
    _PODR(port) |= mask;
    _PXXPFS(port, pin_idx & 7) |= 0x80;
    _PMR(port) |= mask;
    MPC.PWPR.BYTE = 0x80;   /* Disable write to PFSWE and PFS*/
}

void rx_dac_disable(uint8_t pin_idx) {
    uint8_t port = GPIO_PORT(pin_idx);
    uint8_t mask = GPIO_MASK(pin_idx);
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin_idx & 7) &= ~0x80;
    _PMR(port) &= ~mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
}

void rx_dac_init(uint8_t pin_idx) {
    uint8_t channel;
    if ((channel = rx_dac_get_channel(pin_idx)) == 0xff) {
        return;
    }
    SYSTEM.PRCR.WORD = 0xA502;
    SYSTEM.MSTPCRA.BIT.MSTPA19 = 0; // SYSTEM.MSTPCRA.BIT.MSTPA19
    SYSTEM.PRCR.WORD = 0xA500;
    rx_dac_enable(pin_idx);
    if (channel == 0) {
        DA.DACR.BIT.DAOE0 = 1;
    }
    if (channel == 1) {
        DA.DACR.BIT.DAOE1 = 1;
    }
}

void rx_dac_deinit(uint8_t pin_idx) {
    uint8_t channel;
    if ((channel = rx_dac_get_channel(pin_idx)) == 0xff) {
        return;
    }
    rx_dac_disable(pin_idx);
    if (channel == 0) {
        DA.DACR.BIT.DAOE0 = 0;
    }
    if (channel == 1) {
        DA.DACR.BIT.DAOE1 = 0;
    }
    SYSTEM.PRCR.WORD = 0xA502;
    SYSTEM.MSTPCRA.BIT.MSTPA19 = 1; // SYSTEM.MSTPCRA.BIT.MSTPA19
    SYSTEM.PRCR.WORD = 0xA500;
}
