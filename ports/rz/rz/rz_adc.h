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

#ifndef RZ_RZ_AD_H_
#define RZ_RZ_AD_H_

#ifdef __cplusplus
extern "C" {
#endif

int32_t rz_adc_get_resolution(uint32_t pin);
int32_t rz_adc_get_channel(uint32_t pin);
void rz_adc10_enable(uint32_t pin);
void rz_adc12_enable(uint32_t pin);
bool rz_adc_enable(uint32_t pin);
void rz_adc10_disable(uint32_t pin);
void rz_adc12_disable(uint32_t pin);
bool rz_adc_disable(uint32_t pin);
uint16_t rz_adc10_read(uint32_t pin);
uint16_t rz_adc12_read(uint32_t pin);
uint16_t rz_adc_read(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_AD_H_ */
