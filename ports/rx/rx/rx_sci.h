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

#ifndef RX_RX_SCI_H_
#define RX_RX_SCI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rx_gpio.h"

#define SCI_IDX_MAX (11)

#if defined(GRCITRUS)
#define SCI_CH_NUM 7
#define SCI_TX_STATIC_BUF
#define SCI_RX_STATIC_BUF
#define SCI_TX_BUF_SIZE 256
#define SCI_RX_BUF_SIZE 512
#elif defined(GRSAKURA)
#define SCI_CH_NUM 4
#define SCI_TX_STATIC_BUF
#define SCI_RX_STATIC_BUF
#define SCI_TX_BUF_SIZE 256
#define SCI_RX_BUF_SIZE 512
#else
#define SCI_CH_NUM 4
#define SCI_TX_STATIC_BUF
#define SCI_RX_STATIC_BUF
#define SCI_TX_BUF_SIZE 256
#define SCI_RX_BUF_SIZE 512
#endif
#define SCI_DEFAULT_PRIORITY 3
#define SCI_DEFAULT_BAUD    115200

#define rx_SCI_FLOW_START_NUM 16

typedef int (*SCI_CB)(uint32_t ch, uint32_t d);

bool rx_af_find_ch_af(rx_af_pin_t *af_pin, uint32_t size, uint32_t pin, uint32_t *ch, uint32_t *af);
// static void rx_sci_tx_set_pin(uint32_t pin);
// static void rx_sci_rx_set_pin(uint32_t pin);
// static void rx_sci_cts_set_pin(uint32_t pin);
void rx_sci_module(uint32_t ch, int flag);
// static void rx_sci_module_start(uint32_t ch);
// static void rx_sci_module_stop(uint32_t ch);
void rx_sci_rx_set_callback(int ch, SCI_CB cb);
void rx_sci_rx_set_int(uint32_t ch, int flag);
void rx_sci_rx_int_enable(uint32_t ch);
void rx_sci_rx_int_disable(uint32_t ch);
void rx_sci_tx_set_int(uint32_t ch, int flag);
void rx_sci_tx_int_enable(uint32_t ch);
void rx_sci_tx_int_disable(uint32_t ch);
void rx_sci_te_set_int(uint32_t ch, int flag);
void rx_sci_te_int_enable(uint32_t ch);
void rx_sci_te_int_disable(uint32_t ch);
void rx_sci_er_set_int(uint32_t ch, int flag);
void rx_sci_er_int_enable(uint32_t ch);
void rx_sci_er_int_disable(uint32_t ch);
void rx_sci_isr_rx(uint32_t ch);
void rx_sci_isr_er(uint32_t ch);
void rx_sci_isr_tx(uint32_t ch);
void rx_sci_isr_te(uint32_t ch);
int rx_sci_rx_ch(uint32_t ch);
int rx_sci_rx_any(uint32_t ch);
void rx_sci_tx_ch(uint32_t ch, int c);
int rx_sci_tx_wait(uint32_t ch);
void rx_sci_tx_break(uint32_t ch);
void rx_sci_tx_str(uint32_t ch, uint8_t *p);
// static void rx_sci_fifo_set(sci_fifo *fifo, uint8_t *bufp, uint32_t size);
void rx_sci_txfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size);
void rx_sci_rxfifo_set(uint32_t ch, uint8_t *bufp, uint32_t size);
// static void rx_sci_fifo_init(uint32_t ch);
void rx_sci_int_priority(uint32_t ch, int priority);
void rx_sci_int_enable(uint32_t ch);
void rx_sci_int_disable(uint32_t ch);
void rx_sci_set_baud(uint32_t ch, uint32_t baud);
void rx_sci_init_with_flow(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow, uint32_t cts_pin, uint32_t rts_pin);
void rx_sci_init(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow);
void rx_sci_init_default(uint32_t ch, uint32_t tx_pin, uint32_t rx_pin, uint32_t baud);
void rx_sci_deinit(uint32_t ch);

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_SCI_H_ */
