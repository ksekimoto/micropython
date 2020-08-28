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

#ifndef PORTS_RZ_RZA2M_RZA2M_MTU_H_
#define PORTS_RZ_RZA2M_RZA2M_MTU_H_

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

#endif /* PORTS_RZ_RZA2M_RZA2M_MTU_H_ */
