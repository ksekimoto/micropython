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

#ifndef RZ_RZ_SCI_H_
#define RZ_RZ_SCI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rz_gpio.h"

#define RZ_SCI_FLOW_START_NUM   (16)

typedef int (*SCI_CB)(uint32_t ch, uint32_t d);

void rz_sci_rx_set_callback(int ch, SCI_CB cb);
// void rz_sci_rxirq_disable(uint32_t ch);
// void rz_sci_rxirq_enable(uint32_t ch);
// bool rz_sci_is_rxirq_enable(uint32_t ch);
// void rz_sci_isr_te(uint32_t ch);
int rz_sci_rx_ch(uint32_t ch);
int rz_sci_rx_any(uint32_t ch);
void rz_sci_tx_ch(uint32_t ch, int c);
int rz_sci_tx_wait(uint32_t ch);
void rz_sci_tx_break(uint32_t ch);
void rz_sci_tx_str(uint32_t ch, uint8_t *p);
void rz_sci_txfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size);
void rz_sci_rxfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size);
void rz_sci_set_baud(uint32_t ch, uint32_t baud);
void rz_sci_init_with_flow(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow, uint32_t cts_pin, uint32_t rts_pin);
void rz_sci_init(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow);
void rz_sci_init_default(uint32_t ch, uint32_t baud);
void rz_sci_deinit(uint32_t ch);

#ifdef __cplusplus
}
#endif

#endif /* RZ_RZ_SCI_H_ */
