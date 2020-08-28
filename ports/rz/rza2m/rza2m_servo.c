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
#include "rza2m_servo.h"
#include "rza2m_mtu.h"

/*
 * Use TPU PWM Mode2
 * Synchronous pin is TIOCB5 pin
 */

#define RZ_SERVO_DEFAULT_FREQ   (50.0f)
#define RZ_SERVO_UNIT   (1.75f)
#define RZ_SERVO_CH1    5
#define RZ_SERVO_CH2    11
#define RZ_SERVO_INT_PRIORITY   7

//static rz_servo_cb_t rz_servo_cb = NULL;
//static float period = 3750.0f;
//static float unit_10u = 533.3f;

// TPU5 TGI5B
void  __attribute__ ((interrupt)) INT_Excep_TPU5_TGI5B(void) {
#if RZ_TODO
    if (rz_servo_cb) {
        (*rz_servo_cb)();
    }
#endif
}

void rz_servo_int_enable(void) {
#if RZ_TODO
    rz_tpu_int_ipr(TIOCB5 & 0xfe, RZ_SERVO_INT_PRIORITY);
    rz_tpu_int_ier(TIOCB5, 1);
#endif
}

void rz_servo_int_disable(void) {
#if RZ_TODO
    rz_tpu_int_ier(TIOCB5, 0);
#endif
}

void rz_servo_set_callback(rz_servo_cb_t cb) {
#if RZ_TODO
    if (cb == NULL) {
        rz_servo_int_disable();
    }
    rz_servo_cb = cb;
    if (cb != NULL) {
        rz_servo_int_enable();
    }
#endif
}

void rz_servo_enable_it(void) {
#if RZ_TODO
    rz_tpu_channel_int_enable(5, TGRB_BIT);
#endif
}

void rz_servo_disable_it(void) {
#if RZ_TODO
    rz_tpu_channel_int_disable(5, TGRB_BIT);
#endif
}

void rz_servo_set_pulse(uint8_t pin_idx, uint32_t pulse) {
#if RZ_TODO
    float duty = (float)pulse * unit_10u / period;
    rz_tpu_set_duty(pin_idx, duty);
#endif
}

void rz_servo_start(uint8_t pin_idx) {
#if RZ_TODO
    rz_tpu_start(pin_idx);
    //rz_servo_enable_it();
#endif
}

void rz_servo_stop(uint8_t pin_idx) {
#if RZ_TODO
    rz_tpu_stop(pin_idx);
    rz_servo_disable_it();
#endif
}

void rz_servo_init(void) {
#if RZ_TODO
    rz_servo_cb = NULL;
    rz_tpu_init();
    rz_tpu_set_default_freq(RZ_SERVO_DEFAULT_FREQ);
    uint32_t clkdev = rz_tpu_get_clock_dev(RZ_SERVO_CH1, RZ_SERVO_DEFAULT_FREQ);
    period = ((float)PCLK)/(RZ_SERVO_DEFAULT_FREQ * ((float)clkdev));
    unit_10u = (float)(PCLK / 100000) / (float)clkdev;
#endif
}

void rz_servo_deinit(void) {
#if RZ_TODO
    rz_servo_int_disable();
    rz_tpu_deinit();
    rz_servo_cb = NULL;
#endif
}
