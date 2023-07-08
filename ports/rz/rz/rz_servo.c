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
#include "rz_servo.h"
#include "rz_mtu.h"

/*
 * Use TPU PWM Mode2
 * Synchronous pin is TIOCB5 pin
 */

#define RZ_SERVO_DEFAULT_FREQ   (50.0f)
#define RZ_SERVO_UNIT   (1.75f)
#define RZ_SERVO_CH1    5
#define RZ_SERVO_CH2    11
#define RZ_SERVO_INT_PRIORITY   7

// static rz_servo_cb_t rz_servo_cb = NULL;
// static float period = 3750.0f;
// static float unit_10u = 533.3f;

// TPU5 TGI5B
void __attribute__ ((interrupt)) INT_Excep_TPU5_TGI5B(void) {
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
    // rz_servo_enable_it();
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
    period = ((float)PCLK) / (RZ_SERVO_DEFAULT_FREQ * ((float)clkdev));
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
