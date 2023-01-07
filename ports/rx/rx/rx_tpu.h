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

#ifndef RX_RX_TPU_H_
#define RX_RX_TPU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TPU_UNIT_SIZE   2
#define TPU_CHANNEL_SIZE    12
#define TPU_DEFAULT_FREQ    (1000.0f)
#define TPU_DEFAULT_DUTY    (0.1f)

enum TPU_CHANNEL_PIN {
    TIOCA0,
    TIOCB0,
    TIOCC0,
    TIOCD0,
    TIOCA1,
    TIOCB1,
    TIOCA2,
    TIOCB2,
    TIOCA3,
    TIOCB3,
    TIOCC3,
    TIOCD3,
    TIOCA4,
    TIOCB4,
    TIOCA5,
    TIOCB5,
    TIOCA6,
    TIOCB6,
    TIOCC6,
    TIOCD6,
    TIOCA7,
    TIOCB7,
    TIOCA8,
    TIOCB8,
    TIOCA9,
    TIOCB9,
    TIOCC9,
    TIOCD9,
    TIOCA10,
    TIOCB10,
    TIOCA11,
    TIOCB11,
    TPU_END,
};

enum TPU_TGR_BIT {
    TGRA_BIT = 0,
    TGRB_BIT,
    TGRC_BIT,
    TGRD_BIT
};

void rx_tpu_int_ipr(uint8_t tpu_pin, uint8_t priority);
void rx_tpu_int_ier(uint8_t tpu_pin, int flag);
void rx_tpu_channel_int_enable(uint8_t channel, uint8_t bit);
void rx_tpu_channel_int_disable(uint8_t channel, uint8_t bit);
uint8_t rx_tpu_get_tpu_channel(uint8_t pin_idx);
uint32_t rx_tpu_get_clock_dev(int channel, float freq);
void rx_tpu_set_clock(int channel, uint32_t clkdev);
void rx_tpu_set_default_freq(float freq);
void rx_tpu_set_freq(uint8_t pin_idx, float freq);
float rx_tpu_get_freq(uint8_t pin_idx);
void rx_tpu_set_duty(uint8_t pin_idx, float duty);
float rx_tpu_get_duty(uint8_t pin_idx);
void rx_tpu_start(uint8_t pin_idx);
void rx_tpu_stop(uint8_t pin_idx);
void rx_tpu_pin_init_freq(uint8_t pin_idx, float freq, float percent);
void rx_tpu_pin_init(uint8_t pin_idx);
void rx_tpu_pin_deinit(uint8_t pin_idx);
void rx_tpu_init(void);
void rx_tpu_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_TPU_H_ */
