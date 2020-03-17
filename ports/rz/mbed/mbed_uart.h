/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2020 Kentaro Sekimoto
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

#ifndef PORTS_RZ_MBED_MBED_UART_H_
#define PORTS_RZ_MBED_MBED_UART_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*MBED_UART_CB)();
typedef bool (*MBED_UART_KBD_CB)(uint8_t c);

typedef struct _MBED_UART_RX_BUF {
    uint16_t read_buf_len;
    volatile uint16_t read_buf_head;
    uint16_t read_buf_tail;
    uint8_t *read_buf;
} MBED_UART_RX_BUF;

typedef struct _MBED_UART_TX_BUF {
    bool is_sending;
    uint16_t write_buf_len;
    volatile uint16_t write_buf_head;
    uint16_t write_buf_tail;
    uint8_t *write_buf;
} MBED_UART_TX_BUF;

void mbed_uart_init(int ch, int baud, int bits, int parity, int stop, int flow);
void mbed_uart_init_with_pins(int ch, int tx_pin, int rx_pin, int baud, int bits, int parity, int stop, int flow);
void mbed_uart_deinit(int ch);
uint8_t mbed_uart_rx_ch(int ch);
void mbed_uart_tx_ch(int ch, uint8_t c);
int mbed_uart_rx_any(int ch);
bool mbed_uart_tx_wait(int ch);
void mbed_uart_set_kbd_interrupt(int ch, void *ptr);

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_MBED_MBED_UART_H_ */
