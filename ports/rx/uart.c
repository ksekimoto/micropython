/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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
#include <string.h>
#include <stdarg.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "lib/utils/interrupt_char.h"
#include "uart.h"
#include "irq.h"
#include "pendsv.h"
#include "common.h"

/// \moduleref pyb
/// \class UART - duplex serial communication bus
///
/// UART implements the standard UART/USART duplex serial communications protocol.  At
/// the physical level it consists of 2 lines: RX and TX.  The unit of communication
/// is a character (not to be confused with a string character) which can be 8 or 9
/// bits wide.
///
/// UART objects can be created and initialised using:
///
///     from pyb import UART
///
///     uart = UART(1, 9600)                         # init with given baudrate
///     uart.init(9600, bits=8, parity=None, stop=1) # init with given parameters
///
/// Bits can be 8 or 9.  Parity can be None, 0 (even) or 1 (odd).  Stop can be 1 or 2.
///
/// A UART object acts like a stream object and reading and writing is done
/// using the standard stream methods:
///
///     uart.read(10)       # read 10 characters, returns a bytes object
///     uart.read()         # read all available characters
///     uart.readline()     # read a line
///     uart.readinto(buf)  # read and store into the given buffer
///     uart.write('abc')   # write the 3 characters
///
/// Individual characters can be read/written using:
///
///     uart.readchar()     # read 1 character and returns it as an integer
///     uart.writechar(42)  # write 1 character
///
/// To check if there is anything to be read, use:
///
///     uart.any()               # returns True if any characters waiting

extern void NORETURN __fatal_error(const char *msg);
#if MICROPY_KBD_EXCEPTION
extern int mp_interrupt_char;

static int chk_kbd_interrupt(int d)
{
    if (d == mp_interrupt_char) {
        pendsv_kbd_intr();
        return 1;
    } else {
        return 0;
    }
}
#endif

void uart_init0(void) {
    for (uint i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all)); i++) {
        MP_STATE_PORT(pyb_uart_obj_all)[i] = NULL;
    }
}

// unregister all interrupt sources
void uart_deinit_all(void) {
    for (uint i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all)); i++) {
        pyb_uart_obj_t *uart_obj = MP_STATE_PORT(pyb_uart_obj_all)[i];
        if (uart_obj != NULL && !uart_obj->is_static) {
            uart_deinit(uart_obj);
            MP_STATE_PORT(pyb_uart_obj_all)[i] = NULL;
        }
    }
}

bool uart_exists(int uart_id) {
    if (uart_id > MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all))) {
        // safeguard against pyb_uart_obj_all array being configured too small
        return false;
    }
    switch (uart_id) {
        #if defined(MICROPY_HW_UART0_TX) && defined(MICROPY_HW_UART0_RX)
        case PYB_UART_0: return true;
        #endif

        #if defined(MICROPY_HW_UART1_TX) && defined(MICROPY_HW_UART1_RX)
        case PYB_UART_1: return true;
        #endif

        #if defined(MICROPY_HW_UART2_TX) && defined(MICROPY_HW_UART2_RX)
        case PYB_UART_2: return true;
        #endif

        #if defined(MICROPY_HW_UART3_TX) && defined(MICROPY_HW_UART3_RX)
        case PYB_UART_3: return true;
        #endif

        #if defined(MICROPY_HW_UART4_TX) && defined(MICROPY_HW_UART4_RX)
        case PYB_UART_4: return true;
        #endif

        #if defined(MICROPY_HW_UART5_TX) && defined(MICROPY_HW_UART5_RX)
        case PYB_UART_5: return true;
        #endif

        #if defined(MICROPY_HW_UART6_TX) && defined(MICROPY_HW_UART6_RX)
        case PYB_UART_6: return true;
        #endif

        #if defined(MICROPY_HW_UART7_TX) && defined(MICROPY_HW_UART7_RX)
        case PYB_UART_7: return true;
        #endif

        #if defined(MICROPY_HW_UART8_TX) && defined(MICROPY_HW_UART8_RX)
        case PYB_UART_8: return true;
        #endif

        #if defined(MICROPY_HW_UART9_TX) && defined(MICROPY_HW_UART9_RX)
        case PYB_UART_9: return true;
        #endif

        #if defined(MICROPY_HW_UART10_TX) && defined(MICROPY_HW_UART10_RX)
        case PYB_UART_10: return true;
        #endif

        #if defined(MICROPY_HW_UART11_TX) && defined(MICROPY_HW_UART11_RX)
        case PYB_UART_11: return true;
        #endif

        #if defined(MICROPY_HW_UART12_TX) && defined(MICROPY_HW_UART12_RX)
        case PYB_UART_12: return true;
        #endif

        default: return false;
    }
}

// assumes Init parameters have been set up correctly
bool uart_init(pyb_uart_obj_t *uart_obj,
    uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop, uint32_t flow) {
    int uart_unit = (int)uart_obj->uart_id;

    const pin_obj_t *pins[4] = {0};

    switch (uart_obj->uart_id) {
        #if defined(MICROPY_HW_UART0_TX) && defined(MICROPY_HW_UART0_RX)
        case PYB_UART_0:
            uart_unit = 0;
            pins[0] = MICROPY_HW_UART0_TX;
            pins[1] = MICROPY_HW_UART0_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART1_TX) && defined(MICROPY_HW_UART1_RX)
        case PYB_UART_1:
            uart_unit = 1;
            pins[0] = MICROPY_HW_UART1_TX;
            pins[1] = MICROPY_HW_UART1_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART2_TX) && defined(MICROPY_HW_UART2_RX)
        case PYB_UART_2:
            uart_unit = 2;
            pins[0] = MICROPY_HW_UART2_TX;
            pins[1] = MICROPY_HW_UART2_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART3_TX) && defined(MICROPY_HW_UART3_RX)
        case PYB_UART_3:
            uart_unit = 3;
            pins[0] = MICROPY_HW_UART3_TX;
            pins[1] = MICROPY_HW_UART3_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART4_TX) && defined(MICROPY_HW_UART4_RX)
        case PYB_UART_4:
            uart_unit = 4;
            pins[0] = MICROPY_HW_UART4_TX;
            pins[1] = MICROPY_HW_UART4_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART5_TX) && defined(MICROPY_HW_UART5_RX)
        case PYB_UART_5:
            uart_unit = 5;
            pins[0] = MICROPY_HW_UART5_TX;
            pins[1] = MICROPY_HW_UART5_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART6_TX) && defined(MICROPY_HW_UART6_RX)
        case PYB_UART_6:
            uart_unit = 6;
            pins[0] = MICROPY_HW_UART6_TX;
            pins[1] = MICROPY_HW_UART6_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART7_TX) && defined(MICROPY_HW_UART7_RX)
        case PYB_UART_7:
            uart_unit = 7;
            pins[0] = MICROPY_HW_UART7_TX;
            pins[1] = MICROPY_HW_UART7_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART8_TX) && defined(MICROPY_HW_UART8_RX)
        case PYB_UART_8:
            uart_unit = 8;
            pins[0] = MICROPY_HW_UART8_TX;
            pins[1] = MICROPY_HW_UART8_RX;
            break;
        #endif


        #if defined(MICROPY_HW_UART9_TX) && defined(MICROPY_HW_UART9_RX)
        case PYB_UART_9:
            uart_unit = 9;
            pins[0] = MICROPY_HW_UART9_TX;
            pins[1] = MICROPY_HW_UART9_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART10_TX) && defined(MICROPY_HW_UART10_RX)
        case PYB_UART_10:
            uart_unit = 10;
            pins[0] = MICROPY_HW_UART10_TX;
            pins[1] = MICROPY_HW_UART10_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART11_TX) && defined(MICROPY_HW_UART11_RX)
        case PYB_UART_11:
            uart_unit = 11;
            pins[0] = MICROPY_HW_UART11_TX;
            pins[1] = MICROPY_HW_UART11_RX;
            break;
        #endif

        #if defined(MICROPY_HW_UART12_TX) && defined(MICROPY_HW_UART12_RX)
        case PYB_UART_12:
            uart_unit = 12;
            pins[0] = MICROPY_HW_UART12_TX;
            pins[1] = MICROPY_HW_UART12_RX;
            break;
        #endif

        default:
            // UART does not exist or is not configured for this board
            return false;
    }

    sci_init_with_pins(uart_unit, (int)pins[0]->pin, (int)pins[1]->pin, baudrate, bits, parity, stop, flow);

    uart_obj->is_enabled = true;
    uart_obj->attached_to_repl = false;
    return true;
}

void uart_set_rxbuf(pyb_uart_obj_t *self, size_t len, void *buf) {
    self->read_buf_head = 0;
    self->read_buf_tail = 0;
    self->read_buf_len = len;
    self->read_buf = buf;
    //if (len == 0) {
    //    UART_RXNE_IT_DIS(self->uartx);
    //} else {
    //    UART_RXNE_IT_EN(self->uartx);
    //}
}
void uart_deinit(pyb_uart_obj_t *self) {
    self->is_enabled = false;
    sci_deinit(self->uart_id);

    // ToDo: implement
}
void uart_attach_to_repl(pyb_uart_obj_t *self, bool attached) {
    self->attached_to_repl = attached;
#if MICROPY_KBD_EXCEPTION
    if (attached) {
        sci_rx_set_callback((int)self->uart_id, (SCI_CALLBACK)chk_kbd_interrupt);
    } else {
        sci_rx_set_callback((int)self->uart_id, (SCI_CALLBACK)NULL);
    }
#endif
}

mp_uint_t uart_rx_any(pyb_uart_obj_t *self) {
    int ch = (int)self->uart_id;
    return sci_rx_any(ch);
}

// Waits at most timeout milliseconds for at least 1 char to become ready for
// reading (from buf or for direct reading).
// Returns true if something available, false if not.
bool uart_rx_wait(pyb_uart_obj_t *self, uint32_t timeout) {
    int ch = (int)self->uart_id;
    uint32_t start = mtick();
    for (;;) {
        if (sci_rx_any(ch)) {
            return true;
        }
        if (mtick() - start >= timeout) {
            return false; // timeout
        }
        MICROPY_EVENT_POLL_HOOK
    }
}

// assumes there is a character available
int uart_rx_char(pyb_uart_obj_t *self) {
    int ch = (int)self->uart_id;
    return sci_rx_ch(ch);
}

// Waits at most timeout milliseconds for TX register to become empty.
// Returns true if can write, false if can't.
bool uart_tx_wait(pyb_uart_obj_t *self, uint32_t timeout) {
    int ch = (int)self->uart_id;
    uint32_t start = mtick();
    for (;;) {
        if (sci_tx_wait(ch)) {
            return true;
        }
        if (mtick() - start >= timeout) {
            return false; // timeout
        }
        MICROPY_EVENT_POLL_HOOK
    }
}

#if 0
// ToDo: check if this function is needed?
// Waits at most timeout milliseconds for UART flag to be set.
// Returns true if flag is/was set, false on timeout.
STATIC bool uart_wait_flag_set(pyb_uart_obj_t *self, uint32_t flag, uint32_t timeout) {
    // Note: we don't use WFI to idle in this loop because UART tx doesn't generate
    // an interrupt and the flag can be set quickly if the baudrate is large.
    int ch = (int)self->uart_id;
    uint32_t start = mtick();
    for (;;) {
        if (sci_tx_wait(ch)) {
            return true;
        }
        if (timeout == 0 || mtick() - start >= timeout) {
            return false; // timeout
        }
    }
}
#endif

// src - a pointer to the data to send (16-bit aligned for 9-bit chars)
// num_chars - number of characters to send (9-bit chars count for 2 bytes from src)
// *errcode - returns 0 for success, MP_Exxx on error
// returns the number of characters sent (valid even if there was an error)
size_t uart_tx_data(pyb_uart_obj_t *self, const void *src_in, size_t num_chars, int *errcode) {
    int ch = (int)self->uart_id;
    uint8_t *data = (uint8_t *)src_in;
    if (num_chars == 0) {
        *errcode = 0;
        return 0;
    }
    int i;
    for (i = 0; i < (int)num_chars; i++) {
        sci_tx_ch(ch, *data++);
    }
    *errcode = 0;
    return (size_t)i;
}

void uart_tx_strn(pyb_uart_obj_t *uart_obj, const char *str, uint len) {
    int errcode;
    uart_tx_data(uart_obj, str, len, &errcode);
}

