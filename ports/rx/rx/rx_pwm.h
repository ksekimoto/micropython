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

#ifndef RX_RX_PWM_H_
#define RX_RX_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "iodefine.h"

#define PWM_CHANNEL_SIZE    5
#define PWM_DEFAULT_FREQ    (20.0f)
#define PWM_DEFAULT_DUTY    (0.5f)

uint8_t rx_pwm_get_mtu_channel(uint8_t pin_idx);
void rx_pwm_set_freq(uint8_t pin_idx, float freq);
float rx_pwm_get_freq(uint8_t pin_idx);
void rx_pwm_set_duty(uint8_t pin_idx, float duty);
float rx_pwm_get_duty(uint8_t pin_idx);
void rx_pwm_start(uint8_t pin_idx);
void rx_pwm_stop(uint8_t pin_idx);
void rx_pwm_pin_init_freq(uint8_t pin_idx, float freq, float percent);
void rx_pwm_pin_init(uint8_t pin_idx);
void rx_pwm_pin_deinit(uint8_t pin_idx);
void rx_pwm_init(void);
void rx_pwm_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_PWM_H_ */
