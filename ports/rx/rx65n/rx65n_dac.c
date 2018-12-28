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

#include <stdbool.h>
#include "iodefine.h"
#include "rx65n_gpio.h"

static rx_dac_pins[] = {
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

static void rx_dac_enable(uint8_t pin_idx)
{
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
