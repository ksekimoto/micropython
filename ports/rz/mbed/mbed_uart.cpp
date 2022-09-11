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

#include "mbed.h"
#include "Serial.h"
#include "mbed_uart.h"
#include "mpconfigport.h"

// #define MBED_TX_INT_ENABLE

#define SCI_CH_NUM 5
#define SCI_RX_BUF_SIZE    1024
#if defined(MBED_TX_INT_ENABLE)
#define SCI_TX_BUF_SIZE    1024
#endif

static Serial *mbed_uart[SCI_CH_NUM] = {
    (Serial *)0,
    (Serial *)0,
    (Serial *)0,
    (Serial *)0,
    (Serial *)0,
};

static const uint8_t sci_tx_pins[SCI_CH_NUM] = {
    0x42,   /* ch 0 P42 */
    0x73,   /* ch 1 P73 */
    0xe2,   /* ch 2 PE2 */
    0x63,   /* ch 3 P63 */
    0x90    /* ch 4 P90 */
};

static const uint8_t sci_rx_pins[SCI_CH_NUM] = {
    0x41,   /* ch 0 P41 */
    0x71,   /* ch 1 P71 */
    0xe1,   /* ch 2 PE1 */
    0x62,   /* ch 3 P62 */
    0x91    /* ch 4 P91*/
};

static uint8_t rx_buf[SCI_CH_NUM][SCI_RX_BUF_SIZE];
#if defined(MBED_TX_INT_ENABLE)
static uint8_t tx_buf[SCI_CH_NUM][SCI_TX_BUF_SIZE];
#endif

static MBED_UART_RX_BUF mbed_uart_rx_buf[SCI_CH_NUM] = {
    {SCI_RX_BUF_SIZE, 0, 0, (uint8_t *)&rx_buf[0]},
    {SCI_RX_BUF_SIZE, 0, 0, (uint8_t *)&rx_buf[1]},
    {SCI_RX_BUF_SIZE, 0, 0, (uint8_t *)&rx_buf[2]},
    {SCI_RX_BUF_SIZE, 0, 0, (uint8_t *)&rx_buf[3]},
    {SCI_RX_BUF_SIZE, 0, 0, (uint8_t *)&rx_buf[4]},
};

#if defined(MBED_TX_INT_ENABLE)
static MBED_UART_TX_BUF mbed_uart_tx_buf[SCI_CH_NUM] = {
    {false, SCI_TX_BUF_SIZE, 0, 0, (uint8_t *)&tx_buf[0]},
    {false, SCI_TX_BUF_SIZE, 0, 0, (uint8_t *)&tx_buf[1]},
    {false, SCI_TX_BUF_SIZE, 0, 0, (uint8_t *)&tx_buf[2]},
    {false, SCI_TX_BUF_SIZE, 0, 0, (uint8_t *)&tx_buf[3]},
    {false, SCI_TX_BUF_SIZE, 0, 0, (uint8_t *)&tx_buf[4]},
};
#endif

static MBED_UART_KBD_CB mbed_uart_kbd_interrupt[SCI_CH_NUM] = {
    (MBED_UART_KBD_CB)0,
    (MBED_UART_KBD_CB)0,
    (MBED_UART_KBD_CB)0,
    (MBED_UART_KBD_CB)0,
    (MBED_UART_KBD_CB)0,
};

uint8_t mbed_uart_rx_ch(int ch) {
    uint8_t data;
    MBED_UART_RX_BUF *rx_buf = &mbed_uart_rx_buf[ch];
    if (rx_buf->read_buf_tail != rx_buf->read_buf_head) {
        data = rx_buf->read_buf[rx_buf->read_buf_tail];
        rx_buf->read_buf_tail = (rx_buf->read_buf_tail + 1) % rx_buf->read_buf_len;
    } else {
        data = (uint8_t)mbed_uart[ch]->getc();
    }
    return data;
}

void mbed_uart_tx_ch(int ch, uint8_t c) {
    #if defined(MBED_TX_INT_ENABLE)
    MBED_UART_TX_BUF *tx_buf = &mbed_uart_tx_buf[ch];
    if (tx_buf->is_sending) {
        tx_buf->write_buf[tx_buf->write_buf_tail] = c;
        tx_buf->write_buf_tail = (tx_buf->write_buf_tail + 1) % tx_buf->write_buf_len;
    } else {
        tx_buf->is_sending = true;
    }
    #else
    mbed_uart[ch]->putc((int)c);
    #endif
}

int mbed_uart_rx_any(int ch) {
    MBED_UART_RX_BUF *rx_buf = &mbed_uart_rx_buf[ch];
    int buffer_bytes = rx_buf->read_buf_head - rx_buf->read_buf_tail;
    if (buffer_bytes < 0) {
        return buffer_bytes + rx_buf->read_buf_len;
    } else if (buffer_bytes > 0) {
        return buffer_bytes;
    } else {
        return 0;
    }
}

bool mbed_uart_tx_wait(int ch) {
    return !mbed_uart[ch]->writable();
}

#if defined(MBED_TX_INT_ENABLE)
void mbed_uart_tx_int(int ch) {
    MBED_UART_TX_BUF *tx_buf = &mbed_uart_tx_buf[ch];
    if (tx_buf->write_buf_tail != tx_buf->write_buf_head) {
        mbed_uart[ch]->putc((int)tx_buf->write_buf[tx_buf->write_buf_head]);
        tx_buf->write_buf_head = (tx_buf->write_buf_head + 1) % tx_buf->write_buf_len;
    } else {
        tx_buf->is_sending = false;
    }
}

void mbed_uart_tx_int0(void) {
    mbed_uart_tx_int(0);
}

void mbed_uart_tx_int1(void) {
    mbed_uart_tx_int(1);
}

void mbed_uart_tx_int2(void) {
    mbed_uart_tx_int(2);
}

void mbed_uart_tx_int3(void) {
    mbed_uart_tx_int(3);
}

void mbed_uart_tx_int4(void) {
    mbed_uart_tx_int(4);
}

MBED_UART_CB mbed_uart_tx_ints[SCI_CH_NUM] = {
    mbed_uart_tx_int0,
    mbed_uart_tx_int1,
    mbed_uart_tx_int2,
    mbed_uart_tx_int3,
    mbed_uart_tx_int4,
};
#endif

void mbed_uart_rx_int(int ch) {
    // mp_uint_t irq_state = disable_irq();
    MBED_UART_RX_BUF *rx_buf = &mbed_uart_rx_buf[ch];
    uint16_t next_head = (rx_buf->read_buf_head + 1) % rx_buf->read_buf_len;
    uint8_t data = (uint8_t)mbed_uart[ch]->getc();
    if (mbed_uart_kbd_interrupt[ch]) {
        if (mbed_uart_kbd_interrupt[ch]((int)data)) {
            return;
        }
    }
    rx_buf->read_buf[rx_buf->read_buf_head] = data;
    rx_buf->read_buf_head = next_head;
    // enable_irq(irq_state);
}

void mbed_uart_rx_int0(void) {
    mbed_uart_rx_int(0);
}

void mbed_uart_rx_int1(void) {
    mbed_uart_rx_int(1);
}

void mbed_uart_rx_int2(void) {
    mbed_uart_rx_int(2);
}

void mbed_uart_rx_int3(void) {
    mbed_uart_rx_int(3);
}

void mbed_uart_rx_int4(void) {
    mbed_uart_rx_int(4);
}

MBED_UART_CB mbed_uart_rx_ints[SCI_CH_NUM] = {
    mbed_uart_rx_int0,
    mbed_uart_rx_int1,
    mbed_uart_rx_int2,
    mbed_uart_rx_int3,
    mbed_uart_rx_int4,
};

void mbed_uart_set_kbd_interrupt(int ch, void *ptr) {
    mbed_uart_kbd_interrupt[ch] = (MBED_UART_KBD_CB)ptr;
}

void mbed_uart_init_with_pins(int ch, int tx_pin, int rx_pin, int baud, int bits, int parity, int stop, int flow) {
    Serial::Parity _parity;
    switch (parity) {
        case 0:
            _parity = Serial::None;
            break;
        case 1:
            _parity = Serial::Odd;
            break;
        case 2:
            _parity = Serial::Even;
            break;
    }
    mbed_uart[ch] = new Serial((PinName)tx_pin, (PinName)rx_pin, baud);
    mbed_uart[ch]->format(bits, _parity, stop);
    mbed_uart[ch]->attach(mbed_uart_rx_ints[ch], Serial::RxIrq);
    #if defined(MBED_TX_INT_ENABLE)
    mbed_uart[ch]->attach(mbed_uart_tx_ints[ch], Serial::TxIrq);
    #endif
}

void mbed_uart_init(int ch, int baud, int bits, int parity, int stop, int flow) {
    int tx_pin = (int)sci_tx_pins[ch];
    int rx_pin = (int)sci_rx_pins[ch];
    if ((tx_pin != 0xff) && (rx_pin != 0xff)) {
        mbed_uart_init_with_pins(ch, tx_pin, rx_pin, baud, bits, parity, stop, flow);
    }
}

void mbed_uart_deinit(int ch) {
    if (mbed_uart[ch]) {
        delete mbed_uart[ch];
    }
}
