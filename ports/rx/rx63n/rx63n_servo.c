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

#include <stdio.h>
#include "common.h"
#include "rx63n_servo.h"
#include "rx63n_tpu.h"

/*
 * Use TPU PWM Mode2
 * Synchronous pin is TIOCB5 pin
 */

#define RX_SERVO_DEFAULT_FREQ   (50.0f)
#define RX_SERVO_UNIT   (1.75f)
#define RX_SERVO_CH1    5
#define RX_SERVO_CH2    11
#define RX_SERVO_INT_PRIORITY   7

static rx_servo_cb_t rx_servo_cb = NULL;
static float period = 3750.0f;
static float unit_10u = 533.3f;

// TPU5 TGI5B
void  __attribute__ ((interrupt)) INT_Excep_TPU5_TGI5B(void) {
    if (rx_servo_cb) {
        (*rx_servo_cb)();
    }
}

void rx_servo_int_enable(void) {
    rx_tpu_int_ipr(TIOCB5 & 0xfe, RX_SERVO_INT_PRIORITY);
    rx_tpu_int_ier(TIOCB5, 1);
}

void rx_servo_int_disable(void) {
    rx_tpu_int_ier(TIOCB5, 0);
}

void rx_servo_set_callback(rx_servo_cb_t cb) {
    if (cb == NULL) {
        rx_servo_int_disable();
    }
    rx_servo_cb = cb;
    if (cb != NULL) {
        rx_servo_int_enable();
    }
}

void rx_servo_enable_it(void) {
    rx_tpu_channel_int_enable(5, TGRB_BIT);
}

void rx_servo_disable_it(void) {
    rx_tpu_channel_int_disable(5, TGRB_BIT);
}

void rx_servo_set_pulse(uint8_t pin_idx, uint32_t pulse) {
    float duty = (float)pulse * unit_10u / period;
    rx_tpu_set_duty(pin_idx, duty);
}

void rx_servo_start(uint8_t pin_idx) {
    rx_tpu_start(pin_idx);
    //rx_servo_enable_it();
}

void rx_servo_stop(uint8_t pin_idx) {
    rx_tpu_stop(pin_idx);
    rx_servo_disable_it();
}

void rx_servo_init(void) {
    rx_servo_cb = NULL;
    rx_tpu_init();
    rx_tpu_set_default_freq(RX_SERVO_DEFAULT_FREQ);
    uint32_t clkdev = rx_tpu_get_clock_dev(RX_SERVO_CH1, RX_SERVO_DEFAULT_FREQ);
    period = ((float)PCLK)/(RX_SERVO_DEFAULT_FREQ * ((float)clkdev));
    unit_10u = (float)(PCLK / 100000) / (float)clkdev;
}

void rx_servo_deinit(void) {
    rx_servo_int_disable();
    rx_tpu_deinit();
    rx_servo_cb = NULL;
}
