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

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx63n_gpio.h"
#include "rx63n_ad.h"

static uint8_t ad_pin[] = {
    /* AN0 - AN7*/
    114, 115, 116, 117, 118, 119, 110, 111,
    /* AN000 - AN007*/
    32, 33, 34, 35, 36, 37, 38, 39,
    /* AN008 - AN015 */
    104, 105, 106, 107, 108, 109, 72 ,73,
    /* AN0015 - AN020 */
    74, 75, 0, 1, 2, -1
};

static uint8_t ad_pres_bit[] = {
    10, 10, 10, 10, 10, 10, 10, 10,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, -1
};

static uint8_t ad_channel[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, -1
};

int32_t ad_get_pres_bit(uint32_t pin) {
    int i;
    for (i = 0; i < 29; i++) {
        if (ad_pin[i] == (uint8_t)pin) {
            return (uint32_t)ad_pres_bit[i];
        }
    }
    return -1;
}

int32_t ad_get_channel(uint32_t pin) {
    int i;
    for (i = 0; i < 29; i++) {
        if (ad_pin[i] == (uint8_t)pin) {
            return (uint32_t)ad_channel[i];
        }
    }
    return -1;
}

bool ad_enable_sub(uint32_t pin, uint32_t pres_bit) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    uint8_t bit = 1 << (pin & 7);

    if (ad_get_channel(pin) == -1) {
        return false;
    }
    if (!((pres_bit == 10) || (pres_bit ==12))) {
        return false;
    }
    if (pres_bit == 10) {
        SYSTEM.PRCR.WORD = 0xA502;
        /* 10 bit AD start */
        SYSTEM.MSTPCRA.BIT.MSTPA23 = 0;
        SYSTEM.PRCR.WORD = 0xA500;
        /* Enable write to PFSWE */
        MPC.PWPR.BIT.B0WI = 0;
        /* Enable write to PFS */
        MPC.PWPR.BIT.PFSWE = 1;
        _PXXPFS(port, bit) |= 0x80;
        _PMR(port) |= mask;
        /* Disable write to PFSWE and PFS*/
        MPC.PWPR.BYTE = 0x80;
        // software trigger, PCLK, single mode
        AD.ADCR.BYTE = 0x0C;
        // LSB alignment
        AD.ADCR2.BIT.DPSEL = 0;
    } else if (pres_bit == 12) {
        SYSTEM.PRCR.WORD = 0xA502;
        /* 12 bit AD start */
        SYSTEM.MSTPCRA.BIT.MSTPA17 = 0;
        SYSTEM.PRCR.WORD = 0xA500;
        /* Enable write to PFSWE */
        MPC.PWPR.BIT.B0WI = 0;
        MPC.PWPR.BIT.PFSWE = 1;
        _PXXPFS(port, bit) |= 0x80;
        _PMR(port) |= mask;
        /* Disable write to PFSWE and PFS*/
        MPC.PWPR.BYTE = 0x80;
        S12AD.ADCSR.BYTE = 0x0;
        S12AD.ADCER.BIT.ADRFMT = 0;
    }
    return true;
}

bool ad_enable(uint32_t pin) {
    int32_t pres_bit = ad_get_pres_bit(pin);
    if (pres_bit == -1) {
        return false;
    } else {
        ad_enable_sub(pin, (uint32_t)pres_bit);
    }
}

void ad_disable_sub(uint32_t pin, uint32_t pres_bit) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    uint8_t bit = 1 << (pin & 7);

    if (!((pres_bit == 10) || (pres_bit ==12))) {
        return false;
    }
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    if (pres_bit == 10) {
        _PXXPFS(port, bit) &= ~0x80;
        _PMR(port) &= ~mask;
    } else if (pres_bit == 12) {
        _PXXPFS(port, bit) &= ~0x80;
        _PMR(port) &= ~mask;
    }
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
}

void ad_disable(uint32_t pin) {
    int32_t pres_bit = ad_get_pres_bit(pin);
    if (pres_bit == -1) {
        return;
    } else {
        ad_disable_sub(pin, (uint32_t)pres_bit);
    }
}

uint32_t ad_read_sub(uint32_t pin, uint32_t pres_bit, uint32_t channel) {
    int32_t value32 = 0;
    uint16_t off = channel;
    if (pres_bit == 10) {
        AD.ADCSR.BYTE = 0x20 | off;
        while (AD.ADCSR.BIT.ADST);
        value32 = *((unsigned short*)&AD.ADDRA + off);
    } else if (pres_bit == 12) {
        S12AD.ADANS0.WORD |= (1 << off);
        S12AD.ADCSR.BIT.ADST = 1;
        while (S12AD.ADCSR.BIT.ADST);
        value32 = *((unsigned short*)&S12AD.ADDR0 + off);
    }
    return (uint32_t)value32;
}

uint32_t ad_read(uint32_t pin) {
    int32_t pres_bit = ad_get_pres_bit(pin);
    int32_t channel = ad_get_channel(pin);
    int32_t value32 = 0;
    if (pres_bit != -1) {
        value32 = ad_read_sub(pin, (uint32_t)pres_bit, (uint32_t)channel);
    }
    return value32;
}
