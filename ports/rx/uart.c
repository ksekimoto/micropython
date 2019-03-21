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
    for (int i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all)); i++) {
        MP_STATE_PORT(pyb_uart_obj_all)[i] = NULL;
    }
}

// unregister all interrupt sources
void uart_deinit_all(void) {
    for (int i = 0; i < MP_ARRAY_SIZE(MP_STATE_PORT(pyb_uart_obj_all)); i++) {
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
        //MICROPY_EVENT_POLL_HOOK
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
        //MICROPY_EVENT_POLL_HOOK
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

// this IRQ handler is set up to handle RXNE interrupts only
void uart_irq_handler(mp_uint_t uart_id) {
}

/******************************************************************************/
/* MicroPython bindings                                                       */

STATIC void pyb_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (!self->is_enabled) {
        mp_printf(print, "UART(%u)\n", self->uart_id);
    } else {
        mp_printf(print, "UART(%u) is not enabled\n", self->uart_id);
    }
}

/// \method init(baudrate, bits=8, parity=None, stop=1, *, timeout=1000, timeout_char=0, flow=0, read_buf_len=64)
///
/// Initialise the UART bus with the given parameters:
///
///   - `baudrate` is the clock rate.
///   - `bits` is the number of bits per byte, 7, 8 or 9.
///   - `parity` is the parity, `None`, 0 (even) or 1 (odd).
///   - `stop` is the number of stop bits, 1 or 2.
///   - `timeout` is the timeout in milliseconds to wait for the first character.
///   - `timeout_char` is the timeout in milliseconds to wait between characters.
///   - `flow` is RTS | CTS where RTS == 256, CTS == 512
///   - `read_buf_len` is the character length of the read buffer (0 to disable).
STATIC mp_obj_t pyb_uart_init_helper(pyb_uart_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 115200} },
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_parity, MP_ARG_OBJ, {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
        { MP_QSTR_stop, MP_ARG_INT, {.u_int = 1} },
        /* { MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_HWCONTROL_NONE} }, */
        { MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000} },
        { MP_QSTR_timeout_char, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_read_buf_len, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 64} },
    };

    // parse args
    struct {
        mp_arg_val_t baudrate, bits, parity, stop, flow, timeout, timeout_char, read_buf_len;
    } args;
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

    // set the UART configuration values
    self->baud = args.baudrate.u_int;
    uint32_t bits = args.bits.u_int;
    self->bits = bits;
    if (bits != 9) {
        self->char_width = CHAR_WIDTH_8BIT;
    } else {
        self->char_width = CHAR_WIDTH_9BIT;
    }

    uint32_t parity = 0;
    if (args.parity.u_obj == mp_const_none) {
        parity = 0;
    } else {
        mp_int_t p = mp_obj_get_int(args.parity.u_obj);
        parity = (p & 1) ? 1 : 2;
    }

    // init UART (if it fails, it's because the port doesn't exist)
    if (!uart_init(self, args.baudrate.u_int, bits, parity, args.stop.u_int, args.flow.u_int)) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "UART(%d) doesn't exist", self->uart_id));
    }

    // set timeout
    self->timeout = args.timeout.u_int;

    // set timeout_char
    // make sure it is at least as long as a whole character (13 bits to be safe)
    // minimum value is 2ms because sys-tick has a resolution of only 1ms
    self->timeout_char = args.timeout_char.u_int;
    uint32_t min_timeout_char = 13000 / self->baud + 2;
    if (self->timeout_char < min_timeout_char) {
        self->timeout_char = min_timeout_char;
    }
    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct a UART object on the given bus.  `bus` can be 1-6, or 'XA', 'XB', 'YA', or 'YB'.
/// With no additional parameters, the UART object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the UART busses are:
///
///   - `UART(4)` is on `XA`: `(TX, RX) = (X1, X2) = (PA0, PA1)`
///   - `UART(1)` is on `XB`: `(TX, RX) = (X9, X10) = (PB6, PB7)`
///   - `UART(6)` is on `YA`: `(TX, RX) = (Y1, Y2) = (PC6, PC7)`
///   - `UART(3)` is on `YB`: `(TX, RX) = (Y9, Y10) = (PB10, PB11)`
///   - `UART(2)` is on: `(TX, RX) = (X3, X4) = (PA2, PA3)`
STATIC mp_obj_t pyb_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // work out port
    int uart_id = 0;
    if (mp_obj_is_str(args[0])) {
        const char *port = mp_obj_str_get_str(args[0]);
        if (0) {
        #ifdef MICROPY_HW_UART0_NAME
        } else if (strcmp(port, MICROPY_HW_UART0_NAME) == 0) {
            uart_id = PYB_UART_0;
        #endif
        #ifdef MICROPY_HW_UART1_NAME
        } else if (strcmp(port, MICROPY_HW_UART1_NAME) == 0) {
            uart_id = PYB_UART_1;
        #endif
        #ifdef MICROPY_HW_UART2_NAME
        } else if (strcmp(port, MICROPY_HW_UART2_NAME) == 0) {
            uart_id = PYB_UART_2;
        #endif
        #ifdef MICROPY_HW_UART3_NAME
        } else if (strcmp(port, MICROPY_HW_UART3_NAME) == 0) {
            uart_id = PYB_UART_3;
        #endif
        #ifdef MICROPY_HW_UART4_NAME
        } else if (strcmp(port, MICROPY_HW_UART4_NAME) == 0) {
            uart_id = PYB_UART_4;
        #endif
        #ifdef MICROPY_HW_UART5_NAME
        } else if (strcmp(port, MICROPY_HW_UART5_NAME) == 0) {
            uart_id = PYB_UART_5;
        #endif
        #ifdef MICROPY_HW_UART6_NAME
        } else if (strcmp(port, MICROPY_HW_UART6_NAME) == 0) {
            uart_id = PYB_UART_6;
        #endif
        #ifdef MICROPY_HW_UART7_NAME
        } else if (strcmp(port, MICROPY_HW_UART7_NAME) == 0) {
            uart_id = PYB_UART_7;
        #endif
        #ifdef MICROPY_HW_UART8_NAME
        } else if (strcmp(port, MICROPY_HW_UART8_NAME) == 0) {
            uart_id = PYB_UART_8;
        #endif
        #ifdef MICROPY_HW_UART9_NAME
        } else if (strcmp(port, MICROPY_HW_UART9_NAME) == 0) {
            uart_id = PYB_UART_9;
        #endif
        #ifdef MICROPY_HW_UART10_NAME
        } else if (strcmp(port, MICROPY_HW_UART10_NAME) == 0) {
            uart_id = PYB_UART_10;
        #endif
        #ifdef MICROPY_HW_UART11_NAME
        } else if (strcmp(port, MICROPY_HW_UART11_NAME) == 0) {
            uart_id = PYB_UART_11;
        #endif
        #ifdef MICROPY_HW_UART12_NAME
        } else if (strcmp(port, MICROPY_HW_UART12_NAME) == 0) {
            uart_id = PYB_UART_12;
        #endif
        } else {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "UART(%s) doesn't exist", port));
        }
    } else {
        uart_id = mp_obj_get_int(args[0]);
        if (!uart_exists(uart_id)) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "UART(%d) doesn't exist", uart_id));
        }
    }

    pyb_uart_obj_t *self;
    if (MP_STATE_PORT(pyb_uart_obj_all)[uart_id] == NULL) {
        // create new UART object
        self = m_new0(pyb_uart_obj_t, 1);
        self->base.type = &pyb_uart_type;
        self->uart_id = uart_id;
        MP_STATE_PORT(pyb_uart_obj_all)[uart_id] = self;
    } else {
        // reference existing UART object
        self = MP_STATE_PORT(pyb_uart_obj_all)[uart_id];
    }

    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_uart_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t pyb_uart_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_uart_init_helper(MP_OBJ_TO_PTR(args[0]), n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_init_obj, 1, pyb_uart_init);

/// \method deinit()
/// Turn off the UART bus.
STATIC mp_obj_t pyb_uart_deinit(mp_obj_t self_in) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->is_enabled = false;
    sci_deinit((int)self->uart_id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_deinit_obj, pyb_uart_deinit);

/// \method any()
/// Return `True` if any characters waiting, else `False`.
STATIC mp_obj_t pyb_uart_any(mp_obj_t self_in) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(uart_rx_any(self));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_any_obj, pyb_uart_any);

/// \method writechar(char)
/// Write a single character on the bus.  `char` is an integer to write.
/// Return value: `None`.
STATIC mp_obj_t pyb_uart_writechar(mp_obj_t self_in, mp_obj_t char_in) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // get the character to write (might be 9 bits)
    uint16_t data = mp_obj_get_int(char_in);

    // write the character
    int errcode = 0;
    if (uart_tx_wait(self, self->timeout)) {
        uart_tx_data(self, &data, 1, &errcode);
    } else {
        errcode = MP_ETIMEDOUT;
    }

    if (errcode != 0) {
        mp_raise_OSError(errcode);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_uart_writechar_obj, pyb_uart_writechar);

/// \method readchar()
/// Receive a single character on the bus.
/// Return value: The character read, as an integer.  Returns -1 on timeout.
STATIC mp_obj_t pyb_uart_readchar(mp_obj_t self_in) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (uart_rx_wait(self, self->timeout)) {
        return MP_OBJ_NEW_SMALL_INT(uart_rx_char(self));
    } else {
        // return -1 on timeout
        return MP_OBJ_NEW_SMALL_INT(-1);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_readchar_obj, pyb_uart_readchar);

// uart.sendbreak()
STATIC mp_obj_t pyb_uart_sendbreak(mp_obj_t self_in) {
    //pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // ToDo: implementation
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_sendbreak_obj, pyb_uart_sendbreak);

STATIC const mp_rom_map_elem_t pyb_uart_locals_dict_table[] = {
    // instance methods

    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_uart_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pyb_uart_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&pyb_uart_any_obj) },

    /// \method read([nbytes])
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
    /// \method readline()
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj)},
    /// \method readinto(buf[, nbytes])
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    /// \method write(buf)
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },

    { MP_ROM_QSTR(MP_QSTR_writechar), MP_ROM_PTR(&pyb_uart_writechar_obj) },
    { MP_ROM_QSTR(MP_QSTR_readchar), MP_ROM_PTR(&pyb_uart_readchar_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendbreak), MP_ROM_PTR(&pyb_uart_sendbreak_obj) },

    // class constants
    /* { MP_ROM_QSTR(MP_QSTR_RTS), MP_ROM_INT(UART_HWCONTROL_RTS) }, */
    /* { MP_ROM_QSTR(MP_QSTR_CTS), MP_ROM_INT(UART_HWCONTROL_CTS) }, */
};

STATIC MP_DEFINE_CONST_DICT(pyb_uart_locals_dict, pyb_uart_locals_dict_table);

STATIC mp_uint_t pyb_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    byte *buf = buf_in;

    // check that size is a multiple of character width
    if (size & self->char_width) {
        *errcode = MP_EIO;
        return MP_STREAM_ERROR;
    }

    // convert byte size to char size
    size >>= self->char_width;

    // make sure we want at least 1 char
    if (size == 0) {
        return 0;
    }

    // wait for first char to become available
    if (!uart_rx_wait(self, self->timeout)) {
        // return EAGAIN error to indicate non-blocking (then read() method returns None)
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    // read the data
    byte *orig_buf = buf;
    for (;;) {
        int data = uart_rx_char(self);
        if (self->char_width == CHAR_WIDTH_9BIT) {
            *(uint16_t*)buf = data;
            buf += 2;
        } else {
            *buf++ = data;
        }
        if (--size == 0 || !uart_rx_wait(self, self->timeout_char)) {
            // return number of bytes read
            return buf - orig_buf;
        }
    }
}

STATIC mp_uint_t pyb_uart_write(mp_obj_t self_in, const void *buf_in, mp_uint_t size, int *errcode) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const byte *buf = buf_in;

    // check that size is a multiple of character width
    if (size & self->char_width) {
        *errcode = MP_EIO;
        return MP_STREAM_ERROR;
    }

    // wait to be able to write the first character. EAGAIN causes write to return None
    // ToDo: temporarily commented out
    if (!uart_tx_wait(self, self->timeout)) {
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    // write the data
    size_t num_tx = uart_tx_data(self, buf, size >> self->char_width, errcode);

    if (*errcode == 0 || *errcode == MP_ETIMEDOUT) {
        // return number of bytes written, even if there was a timeout
        return num_tx << self->char_width;
    } else {
        return MP_STREAM_ERROR;
    }
}

STATIC mp_uint_t pyb_uart_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    pyb_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_uint_t ret;
    if (request == MP_STREAM_POLL) {
        uintptr_t flags = arg;
        ret = 0;
        if ((flags & MP_STREAM_POLL_RD) && uart_rx_any(self)) {
            ret |= MP_STREAM_POLL_RD;
        }
        //if ((flags & MP_STREAM_POLL_WR) && __HAL_UART_GET_FLAG(&self->uart, UART_FLAG_TXE)) {
        //    ret |= MP_STREAM_POLL_WR;
        //}
    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }
    return ret;
}

STATIC const mp_stream_p_t uart_stream_p = {
    .read = pyb_uart_read,
    .write = pyb_uart_write,
    .ioctl = pyb_uart_ioctl,
    .is_text = false,
};

const mp_obj_type_t pyb_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .print = pyb_uart_print,
    .make_new = pyb_uart_make_new,
    .getiter = mp_identity_getiter,
    .iternext = mp_stream_unbuffered_iter,
    .protocol = &uart_stream_p,
    .locals_dict = (mp_obj_dict_t*)&pyb_uart_locals_dict,
};
