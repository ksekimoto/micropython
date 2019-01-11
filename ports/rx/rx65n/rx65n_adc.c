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

#include <stdio.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx65n_gpio.h"
#include "rx65n_adc.h"

#if defined(RX63N)
#define RX_ADC10
#endif

#if defined(RX_ADC10)
static uint8_t adc10_pin[] = {
    PE2,    /* AN0 */
    PE3,    /* AN1 */
    PE4,    /* AN2 */
    PE5,    /* AN3 */
    PE6,    /* AN4 */
    PE7,    /* AN5 */
    PD6,    /* AN6 */
    PD7,    /* AN7 */
};
#define ADC10_SIZE  (sizeof(adc10_pin)/sizeof(uint8_t))
#endif

static uint8_t adc12_pin[] = {
    P40,    /* AN000 */
    P41,    /* AN001 */
    P42,    /* AN002 */
    P43,    /* AN003 */
    P44,    /* AN004 */
    P45,    /* AN005 */
    P46,    /* AN006 */
    P47,    /* AN007 */
    PD0,    /* AN008 */
    PD1,    /* AN009 */
    PD2,    /* AN010 */
    PD3,    /* AN011 */
    PD4,    /* AN012 */
    PD5,    /* AN013 */
    P90,    /* AN014 */
    P91,    /* AN015 */
    P92,    /* AN016 */
    P93,    /* AN017 */
    P00,    /* AN018 */
    P01,    /* AN019 */
    P02,    /* AN020 */
};
#define ADC12_SIZE  (sizeof(adc12_pin)/sizeof(uint8_t))

int32_t rx_adc_get_resolution(uint8_t pin) {
    int i;
    int res = -1;
#if defined(RX_ADC10)
    for (i = 0; i < ADC10_SIZE; i++) {
        if (adc10_pin[i] == pin) {
            return 10;
        }
    }
#endif
    for (i = 0; i < ADC12_SIZE; i++) {
        if (adc12_pin[i] == pin) {
            return 12;
        }
    }
    return res;
}

int32_t rx_adc_get_channel(uint8_t pin) {
    int i;
    int res = -1;
#if defined(RX_ADC10)
    for (i = 0; i < ADC10_SIZE; i++) {
        if (adc10_pin[i] == pin) {
            return i;
        }
    }
#endif
    for (i = 0; i < ADC12_SIZE; i++) {
        if (adc12_pin[i] == pin) {
            return i;
        }
    }
    return res;
}

#if defined(RX_ADC10)
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
    uint8_t port = GPIO_PORT(pin);
    uint8_t mask = GPIO_MASK(pin);
    SYSTEM.PRCR.WORD = 0xA502;
    /* 12 bit AD start */
    SYSTEM.MSTPCRA.BIT.MSTPA17 = 0;
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
#endif
#if defined(RX65N)
    S12AD.ADCSR.WORD = 0x0;
#endif
    S12AD.ADCER.BIT.ADRFMT = 0;
}

bool rx_adc_enable(uint8_t pin) {
    if (rx_adc_get_channel(pin) == -1) {
        return false;
    }
#if defined(RX_ADC10)
    int resolution;
    resolution = rx_adc_get_resolution(pin);
    if (resolution == 10) {
        rx_adc10_enable(pin);
    } else {
        rx_adc12_enable(pin);
    }
#else
    rx_adc12_enable(pin);
#endif
    return true;
}

#if defined(RX_ADC10)
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
    if (rx_adc_get_channel(pin) == -1) {
        return false;
    }
#if defined(RX_ADC10)
    int resolution;
    resolution = rx_adc_get_resolution(pin);
    if (resolution == 10) {
        rx_adc10_disable(pin);
    } else {
        rx_adc12_disable(pin);
    }
#else
    rx_adc12_disable(pin);
#endif
    return true;
}

#if defined(RX_ADC10)
uint16_t rx_adc10_read(uint8_t pin) {
    uint16_t value16 = 0;
    uint8_t off = (uint8_t)rx_adc_get_channel(pin);
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
    uint16_t off = (uint8_t)rx_adc_get_channel(pin);
#if defined(RX63N)
    S12AD.ADANS0.WORD |= (1 << off);
#endif
#if defined(RX65N)
    S12AD.ADANSA0.WORD |= (1 << off);
#endif
    S12AD.ADCSR.BIT.ADST = 1;
    while (S12AD.ADCSR.BIT.ADST) {
        ;
    }
    value16 = *((unsigned short*)&S12AD.ADDR0 + off);
    return (uint16_t)value16;
}


uint16_t rx_adc_read(uint8_t pin) {
    uint16_t value16;
#if defined(RX_ADC10)
    int resolution = rx_adc_get_resolution(pin);
    if (resolution == 10) {
        value16 = rx_adc10_read(pin);
    } else {
        value16 = rx_adc12_read(pin);
    }
#else
    value16 = rx_adc12_read(pin);
#endif
    return value16;
}
