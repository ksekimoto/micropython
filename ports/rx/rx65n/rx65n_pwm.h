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
#ifndef RX65N_PWM_H_
#define RX65N_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

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

#endif /* RX65N_PWM_H_ */
