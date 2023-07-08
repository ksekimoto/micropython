/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Kentaro Sekimoto
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
#include <stdlib.h>
#include <string.h>
#include "py/runtime.h"
#include "mpy_debug.h"
#include "mpy_uart.h"

#if defined(RZA2M)

#include "rz_sci.h"

#define SCI_INIT_DEFAULT(CH, BAUD)  rz_sci_init(CH, 0x42, 0x41, BAUD, 8, 0, 1, 0)
#define SCI_RX_ANY          rz_sci_rx_any
#define SCI_RX_CH           rz_sci_rx_ch
#define SCI_TX_CH           rz_sci_tx_ch
#define SCI_TX_STR          rz_sci_tx_str

static int esp_ch;
static int esp_baud;

#elif defined(RX63N) || defined(RX65N)

#include "rx_sci.h"

#define SCI_INIT_DEFAULT(CH, BAUD)  rx_sci_init(CH, P23, P25, BAUD, 8, 0, 1, 0)
#define SCI_RX_ANY          rx_sci_rx_any
#define SCI_RX_CH           rx_sci_rx_ch
#define SCI_TX_CH(A, C)     rx_sci_tx_ch(A, C)
#define SCI_TX_STR          rx_sci_tx_str

static int esp_ch;
static int esp_baud;

#elif defined(PICO_BOARD)

#include "py/stream.h"
#include "modmachine.h"

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint32_t id;
} machine_pin_obj_t;
static machine_pin_obj_t tx_pin = { { &machine_pin_type }, 8 };
static machine_pin_obj_t rx_pin = { { &machine_pin_type }, 9 };

extern const mp_obj_type_t machine_uart_type;
static mp_obj_t uart_obj;
static mp_stream_p_t *uart_stream_p;

#else
#endif

#define MAX_DIGITS  20
static volatile bool m_debug_write = false;
static volatile bool m_debug_read = false;

void mpy_uart_debug_write(bool flag) {
    m_debug_write = flag;
}

void mpy_uart_debug_read(bool flag) {
    m_debug_read = flag;
}

static bool isdisplayed(uint8_t c) {
    if ((c == '\r') || (c == '\n') || (0x20 <= c && c <= 0x7e)) {
        return true;
    } else {
        return false;
    }
}

#if defined(RZA2M) || defined(RX63N) || defined(RX65N)
void esp_serial_begin(int uart_id, int baud) {
    #if NEW_IMPL
    mp_esp_uart_obj.base.type = &pyb_uart_type;
    mp_esp_uart_obj.uart_id = uart_id;
    mp_esp_uart_obj.is_static = true;
    // We don't want to block indefinitely, but expect flow control is doing its job.
    mp_esp_uart_obj.timeout = 200;
    mp_esp_uart_obj.timeout_char = 200;
    MP_STATE_PORT(pyb_uart_obj_all)[mp_esp_uart_obj.uart_id - 1] = &mp_esp_uart_obj;
    uart_init(&mp_esp_uart_obj, baud, 8, 0, 1, 0);
    uart_set_rxbuf(&mp_esp_uart_obj, sizeof(esp_rxbuf), esp_rxbuf);
    #else
    esp_ch = uart_id - 1;
    esp_baud = baud;
    SCI_INIT_DEFAULT(esp_ch, esp_baud);
    #endif
}

bool esp_serial_available(void) {
    return (bool)SCI_RX_ANY(esp_ch);
}

uint8_t esp_serial_read_ch(void) {
    uint8_t c = (uint8_t)SCI_RX_CH(esp_ch);
    if (m_debug_read) {
        if (isdisplayed(c)) {
            MPY_DEBUG_TXCH(c);
        } else {
            MPY_DEBUG_TXCH('*');
        }
    }
    return c;
}

uint32_t esp_serial_read_str(uint8_t *buf, size_t size) {
    uint32_t readed = 0;
    uint8_t c = 0;
    while (esp_serial_available()) {
        if (readed == (uint32_t)size) {
            break;
        }
        c = esp_serial_read_ch();
        if (buf != NULL) {
            buf[readed] = c;
            readed++;
        }
    }
    return readed;
}

void esp_serial_write_byte(uint8_t c) {
    SCI_TX_CH(esp_ch, (int)c);
    if (m_debug_write) {
        if (isdisplayed(c)) {
            MPY_DEBUG_TXCH(c);
        } else {
            MPY_DEBUG_TXCH('*');
        }
    }
}

uint32_t esp_serial_write_bytes(uint8_t *buf, size_t size) {
    uint32_t written = 0;
    while (written < (uint32_t)size) {
        esp_serial_write_byte(buf[written]);
        written++;
    }
    return written;
}
#elif defined(PICO_BOARD)
void esp_serial_begin(int uart_id, int baud) {
    mp_obj_t args[] = {
        MP_OBJ_NEW_SMALL_INT(1),
        MP_ROM_QSTR(MP_QSTR_baudrate),
        MP_ROM_INT(115200),
        MP_ROM_QSTR(MP_QSTR_tx),
        MP_ROM_PTR(&tx_pin),
        MP_ROM_QSTR(MP_QSTR_rx),
        MP_ROM_PTR(&rx_pin),
    };
    uart_obj = MP_OBJ_TYPE_GET_SLOT(&machine_uart_type, make_new)(&machine_uart_type, 1, 3, (const mp_obj_t *)args);
    uart_stream_p = (mp_stream_p_t *)MP_OBJ_TYPE_GET_SLOT(&machine_uart_type, protocol);
}

bool esp_serial_available(void) {
    int errcode;
    mp_uint_t ret = uart_stream_p->ioctl(uart_obj, (mp_uint_t)MP_STREAM_POLL, (mp_uint_t)MP_STREAM_POLL_RD, (int *)&errcode);
    (void)errcode;
    return (ret & MP_STREAM_POLL_RD) != 0;
}

uint8_t esp_serial_read_ch(void) {
    uint8_t c;
    int errcode;
    uart_stream_p->read(uart_obj, (void *)&c, (mp_uint_t)1, (int *)&errcode);
    (void)errcode;
    if (m_debug_read) {
        if (isdisplayed(c)) {
            MPY_DEBUG_TXCH(c);
        } else {
            MPY_DEBUG_TXCH('*');
        }
    }
    return c;
}

uint32_t esp_serial_read_str(uint8_t *buf, size_t size) {
    int errcode;
    mp_uint_t readed;
    if (buf != 0) {
        readed = (int)uart_stream_p->read(uart_obj, (void *)buf, (mp_uint_t)size, (int *)&errcode);
    } else {
        for (int i = 0; i < size; i++) {
            esp_serial_read_ch();
        }
        readed = size;
    }
    (void)errcode;
    return (uint32_t)readed;
}

void esp_serial_write_byte(uint8_t c) {
    int errcode;
    uart_stream_p->write(uart_obj, (const void *)&c, (mp_uint_t)1, (int *)&errcode);
    (void)errcode;
    if (m_debug_write) {
        if (isdisplayed(c)) {
            MPY_DEBUG_TXCH(c);
        } else {
            MPY_DEBUG_TXCH('*');
        }
    }
    return;
}

uint32_t esp_serial_write_bytes(uint8_t *buf, size_t size) {
    int errcode;
    mp_uint_t written = (int)uart_stream_p->write(uart_obj, (const void *)buf, (mp_uint_t)size, (int *)&errcode);
    ;
    (void)errcode;
    if (m_debug_write) {
        MPY_DEBUG_TXSTRN((const char *)buf, size);
    }
    return (uint32_t)written;
}
#else
#endif

void esp_serial_str(const char *s) {
    esp_serial_write_bytes((uint8_t *)s, (size_t)strlen(s));
    if (m_debug_write) {
        MPY_DEBUG_TXSTR(s);
    }
}

void esp_serial_str_int(int i) {
    char s[MAX_DIGITS];
    itoa(i, (char *)s, 10);
    esp_serial_str((const char *)s);
    if (m_debug_write) {
        MPY_DEBUG_TXSTR(s);
    }
}

void esp_serial_strln(const char *s) {
    esp_serial_str(s);
    esp_serial_write_byte('\r');
    esp_serial_write_byte('\n');
    if (m_debug_write) {
        MPY_DEBUG_TXCH('\r');
        MPY_DEBUG_TXCH('\n');
    }
}

void esp_serial_str_intln(int i) {
    esp_serial_str_int(i);
    esp_serial_write_byte('\r');
    esp_serial_write_byte('\n');
    if (m_debug_write) {
        MPY_DEBUG_TXCH('\r');
        MPY_DEBUG_TXCH('\n');
    }
}

#if esp_serial_printf
#include "vsnprintf.h"
int esp_serial_printf(const void *format, ...) {
#define ESP_PRINTF_BUF_SIZE 1024
    char buf[ESP_PRINTF_BUF_SIZE];
    int len;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    len = vxsnprintf(buf, (size_t)(ESP_PRINTF_BUF_SIZE - 1), format, arg_ptr);
    if (m_debug_write) {
        MPY_DEBUG_TXSTR((uint8_t *)buf);
    }
    va_end(arg_ptr);
    return len;
}
#endif
