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
#include "rz_gpio.h"
#include "rz_adc.h"

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
#define ADC12_SIZE  (sizeof(adc12_pin) / sizeof(uint32_t))

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
    rz_gpio_mode_af(pin, 1);
    PPMR(port) |= mask1;        /* Peripheral */
    ADC.ADCSR.BIT.ADCS = 0x0;   /* single scan mode */
}

void rz_adc12_disable(uint32_t pin) {
    uint32_t port = GPIO_PORT(pin);
    uint8_t mask1 = GPIO_MASK1(pin);
    rz_gpio_mode_af(pin, 0);
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
