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

#ifndef RZ_RZ_MTU_H_
#define RZ_RZ_MTU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "iodefine.h"

#define PWM_CHANNEL_SIZE    5
#define PWM_DEFAULT_FREQ    (20.0f)
#define PWM_DEFAULT_DUTY    (0.5f)

typedef struct _mtu_map {
    uint16_t pinw;
    uint8_t mtu_id;
    uint8_t af_no;
} mtu_map_t;

uint8_t rz_pwm_get_mtu_channel(uint32_t pin);
void rz_pwm_set_freq(uint32_t pin, float freq);
float rz_pwm_get_freq(uint32_t pin);
void rz_pwm_set_duty(uint32_t pin, float duty);
float rz_pwm_get_duty(uint32_t pin);
void rz_pwm_start(uint32_t pin);
void rz_pwm_stop(uint32_t pin);
void rz_pwm_pin_init_freq(uint32_t pin, float freq, float percent);
void rz_pwm_pin_init(uint32_t pin);
void rz_pwm_pin_deinit(uint32_t pin);
void rz_pwm_init(void);
void rz_pwm_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_MTU_H_ */
