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

#ifndef RX63N_TPU_H_
#define RX63N_TPU_H_

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

#endif /* RX63N_TPU_H_ */
