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

#include <stdio.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx_gpio.h"
#include "rx_adc.h"

static const adc_pin_to_ch_t adc_pin_to_ch[] = {
    #if defined(RX63N)

    { PE2, AN0, 10 },
    { PE3, AN1, 10 },
    { PE4, AN2, 10 },
    { PE5, AN3, 10 },
    { PE6, AN4, 10 },
    { PE7, AN5, 10 },
    { PD6, AN6, 10 },
    { PD7, AN7, 10 },

    { P40, AN000, 12 },
    { P41, AN001, 12 },
    { P42, AN002, 12 },
    { P43, AN003, 12 },
    { P44, AN004, 12 },
    { P45, AN005, 12 },
    { P46, AN006, 12 },
    { P47, AN007, 12 },

    { PE2, AN100, 12 },
    { PE3, AN101, 12 },
    { PE4, AN102, 12 },
    { PE5, AN103, 12 },
    { PE6, AN104, 12 },
    { PE7, AN105, 12 },
    { PD6, AN106, 12 },
    { PD7, AN107, 12 },
    { PD0, AN108, 12 },
    { PD1, AN109, 12 },
    { PD2, AN110, 12 },
    { PD3, AN111, 12 },
    { PD4, AN112, 12 },
    { PD5, AN113, 12 },
    { P90, AN114, 12 },
    { P91, AN115, 12 },
    { P92, AN116, 12 },
    { P93, AN117, 12 },
    { P00, AN118, 12 },
    { P01, AN119, 12 },
    { P02, AN120, 12 },

    #elif defined(RX65N)

    { P40, AN000, 12 },
    { P41, AN001, 12 },
    { P42, AN002, 12 },
    { P43, AN003, 12 },
    { P44, AN004, 12 },
    { P45, AN005, 12 },
    { P46, AN006, 12 },
    { P47, AN007, 12 },

    { PE2, AN100, 12 },
    { PE3, AN101, 12 },
    { PE4, AN102, 12 },
    { PE5, AN103, 12 },
    { PE6, AN104, 12 },
    { PE7, AN105, 12 },
    { PD6, AN106, 12 },
    { PD7, AN107, 12 },
    { PD0, AN108, 12 },
    { PD1, AN109, 12 },
    { PD2, AN110, 12 },
    { PD3, AN111, 12 },
    { PD4, AN112, 12 },
    { PD5, AN113, 12 },
    { P90, AN114, 12 },
    { P91, AN115, 12 },
    { P92, AN116, 12 },
    { P93, AN117, 12 },
    { P00, AN118, 12 },
    { P01, AN119, 12 },
    { P02, AN120, 12 },

    #else
    #error "RX MCU Series is not specified."
    #endif
};
#define ADC_PIN_TO_CH_SIZE (sizeof(adc_pin_to_ch) / sizeof(adc_pin_to_ch_t))

bool rx_adc_pin_to_ch(uint8_t pin, uint8_t *ch, uint8_t *res) {
    uint32_t i;
    *ch = (uint8_t)ADC_NON;
    for (i = 0; i < ADC_PIN_TO_CH_SIZE; i++) {
        if (pin == adc_pin_to_ch[i].pin) {
            *ch = adc_pin_to_ch[i].ch;
            *res = adc_pin_to_ch[i].res;
            break;
        }
    }
    if (*ch == (uint8_t)ADC_NON) {
        return false;
    } else {
        return true;
    }
}

bool rx_adc_ch_to_pin(uint8_t ch, uint8_t *pin, uint8_t *res) {
    uint32_t i;
    *pin = (uint32_t)PIN_END;
    for (i = 0; i < ADC_PIN_TO_CH_SIZE; i++) {
        if (ch == adc_pin_to_ch[i].ch) {
            *pin = adc_pin_to_ch[i].pin;
            *res = adc_pin_to_ch[i].res;
            break;
        }
    }
    if (*pin == (uint32_t)PIN_END) {
        return false;
    } else {
        return true;
    }
}

#if 0
bool rx_adc_chk_res_10(uint32_t pin, uint8_t *ch) {
    uint8_t res;
    bool find = rx_adc_pin_to_ch(pin, ch, &res);
    if (find && (res == 10)) {
        return true;
    } else {
        return false;
    }
}

bool rx_adc_chk_res_12(uint32_t pin, uint8_t *ch) {
    uint8_t res;
    bool find = rx_adc_pin_to_ch(pin, ch, &res);
    if (find && (res == 12)) {
        return true;
    } else {
        return false;
    }
}
#endif

int32_t rx_adc_get_resolution(uint8_t pin) {
    uint8_t ch;
    uint8_t res;
    bool find = rx_adc_pin_to_ch(pin, &ch, &res);
    if (find) {
        return (int32_t)res;
    } else {
        return -1;
    }
}

int32_t rx_adc_get_channel(uint8_t pin) {
    uint8_t ch;
    uint8_t res;
    bool find = rx_adc_pin_to_ch(pin, &ch, &res);
    if (find) {
        return (int32_t)ch;
    } else {
        return -1;
    }
}

#if defined(RX63N)
void rx_adc10_enable(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    SYSTEM.PRCR.WORD = 0xA502;
    /* 10 bit AD start */
    SYSTEM.MSTPCRA.BIT.MSTPA23 = 0;
    SYSTEM.PRCR.WORD = 0xA500;
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) |= 0x80;
    _PMR(port) |= mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
    // software trigger, PCLK, single mode
    AD.ADCR.BYTE = 0x0C;
    // LSB alignment
    AD.ADCR2.BIT.DPSEL = 0;
}
#endif

void rx_adc12_enable(uint8_t pin) {
    uint8_t ch;
    uint8_t res;
    bool find = rx_adc_pin_to_ch(pin, &ch, &res);
    if (!(find && (res == 12))) {
        return;
    }
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    SYSTEM.PRCR.WORD = 0xA502;
    /* 12 bit AD start */
    if ((AN000 <= res) && (res <= AN007)) {
        if (SYSTEM.MSTPCRA.BIT.MSTPA17 == 1) {
            SYSTEM.MSTPCRA.BIT.MSTPA17 = 0;
        }
    }
    #if defined(RX65N)
    if ((AN100 <= res) && (res <= AN120)) {
        if (SYSTEM.MSTPCRA.BIT.MSTPA16 == 1) {
            SYSTEM.MSTPCRA.BIT.MSTPA16 = 0;
        }
    }
    #endif
    SYSTEM.PRCR.WORD = 0xA500;
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) |= 0x80;
    _PMR(port) |= mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
    #if defined(RX63N)
    S12AD.ADCSR.BYTE = 0x0;
    S12AD.ADCER.BIT.ADRFMT = 0;
    #endif
    #if defined(RX65N)
    if ((AN000 <= res) && (res <= AN007)) {
        S12AD.ADCSR.WORD = 0x0;
        S12AD.ADCER.BIT.ADRFMT = 0;
    }
    if ((AN100 <= res) && (res <= AN120)) {
        S12AD1.ADCSR.WORD = 0x0;
        S12AD1.ADCER.BIT.ADRFMT = 0;
    }
    #endif
}

bool rx_adc_enable(uint8_t pin) {
    int32_t res = rx_adc_get_resolution(pin);
    if (res == -1) {
        return false;
    }
    #if defined(RX63N)
    if (res == 10) {
        rx_adc10_enable(pin);
    }
    #endif
    if (res == 12) {
        rx_adc12_enable(pin);
    }
    return true;
}

#if defined(RX63N)
void rx_adc10_disable(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) &= ~0x80;
    _PMR(port) &= ~mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
}
#endif

void rx_adc12_disable(uint8_t pin) {
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    /* Enable write to PFSWE */
    MPC.PWPR.BIT.B0WI = 0;
    /* Enable write to PFS */
    MPC.PWPR.BIT.PFSWE = 1;
    _PXXPFS(port, pin & 7) &= ~0x80;
    _PMR(port) &= ~mask;
    /* Disable write to PFSWE and PFS*/
    MPC.PWPR.BYTE = 0x80;
}

bool rx_adc_disable(uint8_t pin) {
    int32_t res = rx_adc_get_resolution(pin);
    if (res == -1) {
        return false;
    #if defined(RX63N)
    } else if (res == 10) {
        rx_adc10_disable(pin);
    #endif
    } else {
        rx_adc12_disable(pin);
    }
    return true;
}

#if defined(RX63N)
uint16_t rx_adc10_read(uint8_t pin) {
    uint16_t value16 = 0;
    uint8_t off = (uint8_t)(rx_adc_get_channel(pin) - (uint8_t)AN0);
    AD.ADCSR.BYTE = 0x20 | off;
    while (AD.ADCSR.BIT.ADST) {
        ;
    }
    value16 = *((uint16_t *)&AD.ADDRA + off);
    return (uint16_t)value16;
}
#endif

uint16_t rx_adc12_read(uint8_t pin) {
    uint16_t value16 = 0;
    #if defined(RX63N)
    uint16_t off = (uint8_t)(rx_adc_get_channel(pin) - (uint8_t)AN0);
    S12AD.ADANS0.WORD |= (1 << off);
    S12AD.ADCSR.BIT.ADST = 1;
    while (S12AD.ADCSR.BIT.ADST) {
        ;
    }
    value16 = *((unsigned short *)&S12AD.ADDR0 + off);
    #elif defined(RX65N)
    uint16_t off;
    uint8_t res = rx_adc_get_channel(pin);
    if ((AN000 <= res) && (res <= AN007)) {
        off = (uint16_t)(res - (uint8_t)AN000);
        S12AD.ADANSA0.WORD |= (1 << off);
        S12AD.ADCSR.BIT.ADST = 1;
        while (S12AD.ADCSR.BIT.ADST) {
            ;
        }
        value16 = *((unsigned short *)&S12AD.ADDR0 + off);
    } else {
        off = (uint16_t)(res - (uint8_t)AN100);
        S12AD1.ADANSA0.WORD |= (1 << off);
        S12AD1.ADCSR.BIT.ADST = 1;
        while (S12AD1.ADCSR.BIT.ADST) {
            ;
        }
        value16 = *((unsigned short *)&S12AD1.ADDR0 + off);
    }
    #else
    #error "RX MCU Series is not specified."
    #endif
    return (uint16_t)value16;
}

uint16_t rx_adc_read(uint8_t pin) {
    uint16_t value16 = 0;
    int res = rx_adc_get_resolution(pin);
    #if defined(RX63N)
    if (res == 10) {
        value16 = rx_adc10_read(pin);
    }
    #endif
    if (res == 12) {
        value16 = rx_adc12_read(pin);
    }
    return value16;
}
