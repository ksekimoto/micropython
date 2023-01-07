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

#ifndef RX_RX_AD_H_
#define RX_RX_AD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum ADC_PIN
{
    AN0 = 0,
    AN1,
    AN2,
    AN3,
    AN4,
    AN5,
    AN6,
    AN7,
    AN000 = 32,
    AN001,
    AN002,
    AN003,
    AN004,
    AN005,
    AN006,
    AN007,
    AN100 = 64,
    AN101,
    AN102,
    AN103,
    AN104,
    AN105,
    AN106,
    AN107,
    AN108,
    AN109,
    AN110,
    AN111,
    AN112,
    AN113,
    AN114,
    AN115,
    AN116,
    AN117,
    AN118,
    AN119,
    AN120,
    ADC_NON = 255,
};

typedef struct adc_pin_to_ch {
    uint8_t pin;
    uint8_t ch;
    uint8_t res;
} adc_pin_to_ch_t;

bool rx_adc_pin_to_ch(uint8_t pin, uint8_t *ch, uint8_t *res);
bool rx_adc_ch_to_pin(uint8_t ch, uint8_t *pin, uint8_t *res);
bool rx_adc_chk_res_10(uint8_t pin, uint8_t *ch);
bool rx_adc_chk_res_12(uint8_t pin, uint8_t *ch);
int32_t rx_adc_get_resolution(uint8_t pin);
int32_t rx_adc_get_channel(uint8_t pin);
void rx_adc10_enable(uint8_t pin);
void rx_adc12_enable(uint8_t pin);
bool rx_adc_enable(uint8_t pin);
void rx_adc10_disable(uint8_t pin);
void rx_adc12_disable(uint8_t pin);
bool rx_adc_disable(uint8_t pin);
uint16_t rx_adc10_read(uint8_t pin);
uint16_t rx_adc12_read(uint8_t pin);
uint16_t rx_adc_read(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_AD_H_ */
