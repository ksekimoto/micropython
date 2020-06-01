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

#include <stdio.h>
#include "common.h"
#include "iodefine.h"
#include "rza2m_gpio.h"
#include "rza2m_adc.h"

#define RX_ADC12

static uint32_t adc12_pin[] = {
    P50,    /* AN000 */
    P51,    /* AN001 */
    P52,    /* AN002 */
    P53,    /* AN003 */
    P54,    /* AN004 */
    P55,    /* AN005 */
    P56,    /* AN006 */
    P57,    /* AN007 */
};
#define ADC12_SIZE  (sizeof(adc12_pin)/sizeof(uint32_t))

static volatile uint16_t *ADDR[] = {
    &ADC.ADDR0.WORD,
    &ADC.ADDR1.WORD,
    &ADC.ADDR2.WORD,
    &ADC.ADDR3.WORD,
    &ADC.ADDR4.WORD,
    &ADC.ADDR5.WORD,
    &ADC.ADDR6.WORD,
    &ADC.ADDR7.WORD,
};

int32_t rz_adc_get_resolution(uint32_t pin) {
    int i;
    int res = -1;
    for (i = 0; i < ADC12_SIZE; i++) {
        if (adc12_pin[i] == pin) {
            return 10;
        }
    }
    return res;
}

int32_t rz_adc_get_channel(uint32_t pin) {
    int i;
    int ch = -1;
    for (i = 0; i < ADC12_SIZE; i++) {
        if (adc12_pin[i] == pin) {
            return i;
        }
    }
    return ch;
}

void rz_adc12_enable(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    CPG.STBCR5.BIT.MSTP57 = 0;
    _gpio_mode_af(pin, 1);
    PPMR(port) |= mask1;        /* Peripheral */
    ADC.ADCSR.BIT.ADCS = 0x0;   /* single scan mode */
}

void rz_adc12_disable(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    _gpio_mode_af(pin, 0);
    PPMR(port) &= ~mask1;       /* GPIO */
    ADC.ADCSR.BIT.ADCS = 0x0;   /* single scan mode */
}

bool rz_adc_enable(uint32_t pin) {
    if (rz_adc_get_channel(pin) == -1) {
        return false;
    }
    rz_adc12_enable(pin);
    return true;
}

bool rz_adc_disable(uint32_t pin) {
    if (rz_adc_get_channel(pin) == -1) {
        return false;
    }
    rz_adc12_disable(pin);
    return true;
}

uint16_t rz_adc12_read(uint32_t pin) {
    uint32_t ch = rz_adc_get_channel(pin);

    ADC.ADANSA0.WORD = (1 << ch);
    ADC.ADCSR.BIT.ADST = 0x1;
    // Wait end of conversion
    while (ADC.ADCSR.BIT.ADST != 0) {
        ;
    }
    return *(ADDR[ch]) & 0x0FFF;    /* 12 bits range */
}

uint16_t rz_adc_read(uint32_t pin) {
    return rz_adc12_read(pin);
}
