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

#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/machine_spi.h"
#include "pin.h"
#include "bufhelper.h"
#include "spi.h"
#include "common.h"

/// \moduleref pyb
/// \class SPI - a master-driven serial protocol
///
/// SPI is a serial protocol that is driven by a master.  At the physical level
/// there are 3 lines: SCK, MOSI, MISO.
///
/// See usage model of I2C; SPI is very similar.  Main difference is
/// parameters to init the SPI bus:
///
///     from pyb import SPI
///     spi = SPI(1, SPI.MASTER, baudrate=600000, polarity=1, phase=0, crc=0x7)
///
/// Only required parameter is mode, SPI.MASTER or SPI.SLAVE.  Polarity can be
/// 0 or 1, and is the level the idle clock line sits at.  Phase can be 0 or 1
/// to sample data on the first or second clock edge respectively.  Crc can be
/// None for no CRC, or a polynomial specifier.
///
/// Additional method for SPI:
///
///     data = spi.send_recv(b'1234')        # send 4 bytes and receive 4 bytes
///     buf = bytearray(4)
///     spi.send_recv(b'1234', buf)          # send 4 bytes and receive 4 into buf
///     spi.send_recv(buf, buf)              # send/recv 4 bytes from/to buf

/* So far, only ch is used. */
const spi_t spi_obj[3] = {
};

void spi_init0(void) {
}

STATIC int spi_find(mp_obj_t id) {
    if (MP_OBJ_IS_STR(id)) {
        // given a string id
        const char *port = mp_obj_str_get_str(id);
        if (0) {
        #ifdef MICROPY_HW_SPI0_NAME
        } else if (strcmp(port, MICROPY_HW_SPI0_NAME) == 0) {
            return 0;
        #endif
        #ifdef MICROPY_HW_SPI1_NAME
        } else if (strcmp(port, MICROPY_HW_SPI1_NAME) == 0) {
            return 1;
        #endif
        #ifdef MICROPY_HW_SPI2_NAME
        } else if (strcmp(port, MICROPY_HW_SPI2_NAME) == 0) {
            return 2;
        #endif
        }
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
            "SPI(%s) doesn't exist", port));
    } else {
        // given an integer id
        int spi_id = mp_obj_get_int(id);
        if (spi_id >= 0 && spi_id <= MP_ARRAY_SIZE(spi_obj)) {
            return spi_id;
        }
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
            "SPI(%d) doesn't exist", spi_id));
    }
}

// sets the parameters in the SPI_InitTypeDef struct
// if an argument is -1 then the corresponding parameter is not changed
STATIC void spi_set_params(const spi_t *spi_obj, uint32_t prescale, int32_t baudrate,
    int32_t polarity, int32_t phase, int32_t bits, int32_t firstbit) {
    rx_spi_set_spi_ch(spi_obj->ch, polarity, phase);
    rx_spi_set_clk(spi_obj->ch, baudrate);
    rx_spi_set_bits(spi_obj->ch, bits);
    rx_spi_set_firstbit(spi_obj->ch, firstbit);
}

// TODO allow to take a list of pins to use
void spi_init(const spi_t *self, bool enable_nss_pin) {
    const pin_obj_t *pins[4] = { NULL, NULL, NULL, NULL };

    if (0) {
    #if defined(MICROPY_HW_SPI0_SCK)
    } else if (self->ch == 0) {
        #if defined(MICROPY_HW_SPI0_NSS)
        pins[0] = MICROPY_HW_SPI0_NSS;
        #endif
        pins[1] = MICROPY_HW_SPI0_SCK;
        #if defined(MICROPY_HW_SPI3_MISO)
        pins[2] = MICROPY_HW_SPI0_MISO;
        #endif
        pins[3] = MICROPY_HW_SPI0_MOSI;
        rx_spi_init(self->ch, 0, 4000000, 8, 0);
    #endif
    #if defined(MICROPY_HW_SPI1_SCK)
    } else if (self->ch == 1) {
        #if defined(MICROPY_HW_SPI1_NSS)
        pins[0] = MICROPY_HW_SPI1_NSS;
        #endif
        pins[1] = MICROPY_HW_SPI1_SCK;
        #if defined(MICROPY_HW_SPI1_MISO)
        pins[2] = MICROPY_HW_SPI1_MISO;
        #endif
        pins[3] = MICROPY_HW_SPI1_MOSI;
        rx_spi_init(self->ch, 0, 4000000, 8, 0);
    #endif
    #if defined(MICROPY_HW_SPI2_SCK)
    } else if (self->ch == 2) {
        #if defined(MICROPY_HW_SPI2_NSS)
        pins[0] = MICROPY_HW_SPI2_NSS;
        #endif
        pins[1] = MICROPY_HW_SPI2_SCK;
        #if defined(MICROPY_HW_SPI2_MISO)
        pins[2] = MICROPY_HW_SPI2_MISO;
        #endif
        pins[3] = MICROPY_HW_SPI2_MOSI;
        rx_spi_init(self->ch, 0, 4000000, 8, 0);
    #endif
    } else {
        // SPI does not exist for this board (shouldn't get here, should be checked by caller)
        return;
    }

}

void spi_deinit(const spi_t *spi_obj) {
    if (0) {
    #if defined(MICROPY_HW_SPI0_SCK)
    } else if (spi_obj->ch == 0) {
    #endif
    #if defined(MICROPY_HW_SPI1_SCK)
    } else if (spi_obj->ch == 1) {
    #endif
    #if defined(MICROPY_HW_SPI2_SCK)
    } else if (spi_obj->ch == 2) {
    #endif
    }
    rx_spi_deinit(spi_obj->ch, 0);
}

// A transfer of "len" bytes should take len*8*1000/baudrate milliseconds.
// To simplify the calculation we assume the baudrate is never less than 8kHz
// and use that value for the baudrate in the formula, plus a small constant.
#define SPI_TRANSFER_TIMEOUT(len) ((len) + 100)

STATIC void spi_transfer(const spi_t *self, size_t len, const uint8_t *src, uint8_t *dest, uint32_t timeout) {
    // Note: there seems to be a problem sending 1 byte using DMA the first
    // time directly after the SPI/DMA is initialised.  The cause of this is
    // unknown but we sidestep the issue by using polling for 1 byte transfer.
    rx_spi_transfer(self->ch, dest, src, (uint32_t)len, timeout);
}

STATIC void spi_print(const mp_print_t *print, const spi_t *spi_obj, bool legacy) {
    uint spi_num = 1; // default to SPI1
    mp_printf(print, "SPI(%u", spi_num);
    mp_print_str(print, ")");
}

/******************************************************************************/
/* MicroPython bindings for legacy pyb API                                    */

typedef struct _pyb_spi_obj_t {
    mp_obj_base_t base;
    const spi_t *spi;
} pyb_spi_obj_t;

STATIC const pyb_spi_obj_t pyb_spi_obj[] = {
    {{&pyb_spi_type}, &spi_obj[0]},
    {{&pyb_spi_type}, &spi_obj[1]},
    {{&pyb_spi_type}, &spi_obj[2]},
};

STATIC void pyb_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    spi_print(print, self->spi, true);
}


/// \method init(mode, baudrate=328125, *, polarity=1, phase=0, bits=8, firstbit=SPI.MSB, ti=False, crc=None)
///
/// Initialise the SPI bus with the given parameters:
///
///   - `mode` must be either `SPI.MASTER` or `SPI.SLAVE`.
///   - `baudrate` is the SCK clock rate (only sensible for a master).
STATIC mp_obj_t pyb_spi_init_helper(const pyb_spi_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 328125} },
        { MP_QSTR_prescaler, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0xffffffff} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 1} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 0} },
        { MP_QSTR_dir,      MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = SPI_DIRECTION_2LINES} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 8} },
        { MP_QSTR_nss,      MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = SPI_NSS_SOFT} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = SPI_FIRSTBIT_MSB} },
        { MP_QSTR_ti,       MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_crc,      MP_ARG_KW_ONLY | MP_ARG_OBJ,  {.u_rom_obj = MP_ROM_PTR(&mp_const_none_obj)} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    spi_set_params(self->spi, args[2].u_int, args[1].u_int, args[3].u_int, args[4].u_int,
        args[6].u_int, args[8].u_int);
    uint32_t nss = args[7].u_int;
    // init the SPI bus
    spi_init(self->spi, nss != SPI_NSS_SOFT);

    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct an SPI object on the given bus.  `bus` can be 1 or 2.
/// With no additional parameters, the SPI object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the SPI busses are:
///
///   - `SPI(1)` is on the X position: `(NSS, SCK, MISO, MOSI) = (X5, X6, X7, X8) = (PA4, PA5, PA6, PA7)`
///   - `SPI(2)` is on the Y position: `(NSS, SCK, MISO, MOSI) = (Y5, Y6, Y7, Y8) = (PB12, PB13, PB14, PB15)`
///
/// At the moment, the NSS pin is not used by the SPI driver and is free
/// for other use.
STATIC mp_obj_t pyb_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // work out SPI bus
    int spi_id = spi_find(args[0]);

    // get SPI object
    const pyb_spi_obj_t *spi_obj = &pyb_spi_obj[spi_id - 1];

    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_spi_init_helper(spi_obj, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(spi_obj);
}

STATIC mp_obj_t pyb_spi_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_spi_init_helper(MP_OBJ_TO_PTR(args[0]), n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_spi_init_obj, 1, pyb_spi_init);

/// \method deinit()
/// Turn off the SPI bus.
STATIC mp_obj_t pyb_spi_deinit(mp_obj_t self_in) {
    pyb_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    spi_deinit(self->spi);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_spi_deinit_obj, pyb_spi_deinit);

/// \method send(send, *, timeout=5000)
/// Send data on the bus:
///
///   - `send` is the data to send (an integer to send, or a buffer object).
///   - `timeout` is the timeout in milliseconds to wait for the send.
///
/// Return value: `None`.
STATIC mp_obj_t pyb_spi_send(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits wide

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_send,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_spi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get the buffer to send from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(args[0].u_obj, &bufinfo, data);

    // send the data
    spi_transfer(self->spi, bufinfo.len, bufinfo.buf, NULL, args[1].u_int);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_spi_send_obj, 1, pyb_spi_send);

/// \method recv(recv, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC mp_obj_t pyb_spi_recv(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits wide

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_recv,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_spi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get the buffer to receive into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(args[0].u_obj, &vstr);

    // receive the data
    spi_transfer(self->spi, vstr.len, NULL, (uint8_t*)vstr.buf, args[1].u_int);

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return o_ret;
    } else {
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_spi_recv_obj, 1, pyb_spi_recv);

/// \method send_recv(send, recv=None, *, timeout=5000)
///
/// Send and receive data on the bus at the same time:
///
///   - `send` is the data to send (an integer to send, or a buffer object).
///   - `recv` is a mutable buffer which will be filled with received bytes.
///   It can be the same as `send`, or omitted.  If omitted, a new buffer will
///   be created.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: the buffer with the received bytes.
STATIC mp_obj_t pyb_spi_send_recv(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits wide

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_send,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_recv,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_spi_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get buffers to send from/receive to
    mp_buffer_info_t bufinfo_send;
    uint8_t data_send[1];
    mp_buffer_info_t bufinfo_recv;
    vstr_t vstr_recv;
    mp_obj_t o_ret;

    if (args[0].u_obj == args[1].u_obj) {
        // same object for send and receive, it must be a r/w buffer
        mp_get_buffer_raise(args[0].u_obj, &bufinfo_send, MP_BUFFER_RW);
        bufinfo_recv = bufinfo_send;
        o_ret = args[0].u_obj;
    } else {
        // get the buffer to send from
        pyb_buf_get_for_send(args[0].u_obj, &bufinfo_send, data_send);

        // get the buffer to receive into
        if (args[1].u_obj == MP_OBJ_NULL) {
            // only send argument given, so create a fresh buffer of the send length
            vstr_init_len(&vstr_recv, bufinfo_send.len);
            bufinfo_recv.len = vstr_recv.len;
            bufinfo_recv.buf = vstr_recv.buf;
            o_ret = MP_OBJ_NULL;
        } else {
            // recv argument given
            mp_get_buffer_raise(args[1].u_obj, &bufinfo_recv, MP_BUFFER_WRITE);
            if (bufinfo_recv.len != bufinfo_send.len) {
                mp_raise_ValueError("recv must be same length as send");
            }
            o_ret = args[1].u_obj;
        }
    }

    // do the transfer
    spi_transfer(self->spi, bufinfo_send.len, bufinfo_send.buf, bufinfo_recv.buf, args[2].u_int);

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return o_ret;
    } else {
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr_recv);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_spi_send_recv_obj, 1, pyb_spi_send_recv);

STATIC const mp_rom_map_elem_t pyb_spi_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_spi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pyb_spi_deinit_obj) },

    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_machine_spi_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_machine_spi_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_machine_spi_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_readinto), MP_ROM_PTR(&mp_machine_spi_write_readinto_obj) },

    // legacy methods
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&pyb_spi_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&pyb_spi_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_send_recv), MP_ROM_PTR(&pyb_spi_send_recv_obj) },

    // class constants
    /// \constant MASTER - for initialising the bus to master mode
    /// \constant SLAVE - for initialising the bus to slave mode
    /// \constant MSB - set the first bit to MSB
    /// \constant LSB - set the first bit to LSB
    { MP_ROM_QSTR(MP_QSTR_MASTER), MP_ROM_INT(SPI_MODE_MASTER) },
    { MP_ROM_QSTR(MP_QSTR_SLAVE),  MP_ROM_INT(SPI_MODE_SLAVE) },
    { MP_ROM_QSTR(MP_QSTR_MSB),    MP_ROM_INT(SPI_FIRSTBIT_MSB) },
    { MP_ROM_QSTR(MP_QSTR_LSB),    MP_ROM_INT(SPI_FIRSTBIT_LSB) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_spi_locals_dict, pyb_spi_locals_dict_table);

STATIC void spi_transfer_machine(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    pyb_spi_obj_t *self = (pyb_spi_obj_t*)self_in;
    spi_transfer(self->spi, len, src, dest, SPI_TRANSFER_TIMEOUT(len));
}

STATIC const mp_machine_spi_p_t pyb_spi_p = {
    .transfer = spi_transfer_machine,
};

const mp_obj_type_t pyb_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .print = pyb_spi_print,
    .make_new = pyb_spi_make_new,
    .protocol = &pyb_spi_p,
    .locals_dict = (mp_obj_dict_t*)&pyb_spi_locals_dict,
};

/******************************************************************************/
// Implementation of hard SPI for machine module

typedef struct _machine_hard_spi_obj_t {
    mp_obj_base_t base;
    const spi_t *spi;
} machine_hard_spi_obj_t;

STATIC const machine_hard_spi_obj_t machine_hard_spi_obj[] = {
    {{&machine_hard_spi_type}, &spi_obj[0]},
    {{&machine_hard_spi_type}, &spi_obj[1]},
    {{&machine_hard_spi_type}, &spi_obj[2]},
};

STATIC void machine_hard_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hard_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    spi_print(print, self->spi, false);
}

mp_obj_t machine_hard_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_id, ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit, ARG_sck, ARG_mosi, ARG_miso };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(-1)} },
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 500000} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = SPI_FIRSTBIT_MSB} },
        { MP_QSTR_sck,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mosi,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_miso,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get static peripheral object
    int spi_id = spi_find(args[ARG_id].u_obj);
    const machine_hard_spi_obj_t *self = &machine_hard_spi_obj[spi_id - 1];

    // here we would check the sck/mosi/miso pins and configure them, but it's not implemented
    if (args[ARG_sck].u_obj != MP_OBJ_NULL
        || args[ARG_mosi].u_obj != MP_OBJ_NULL
        || args[ARG_miso].u_obj != MP_OBJ_NULL) {
        mp_raise_ValueError("explicit choice of sck/mosi/miso is not implemented");
    }

    // set the SPI configuration values
    // set configurable paramaters
    spi_set_params(self->spi, 0xffffffff, args[ARG_baudrate].u_int,
        args[ARG_polarity].u_int, args[ARG_phase].u_int, args[ARG_bits].u_int,
        args[ARG_firstbit].u_int);

    // init the SPI bus
    spi_init(self->spi, false);

    return MP_OBJ_FROM_PTR(self);
}

STATIC void machine_hard_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;

    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set the SPI configuration values
    spi_set_params(self->spi, 0xffffffff, args[ARG_baudrate].u_int,
        args[ARG_polarity].u_int, args[ARG_phase].u_int, args[ARG_bits].u_int,
        args[ARG_firstbit].u_int);

    // re-init the SPI bus
    spi_init(self->spi, false);
}

STATIC void machine_hard_spi_deinit(mp_obj_base_t *self_in) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;
    spi_deinit(self->spi);
}

STATIC void machine_hard_spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    machine_hard_spi_obj_t *self = (machine_hard_spi_obj_t*)self_in;
    spi_transfer(self->spi, len, src, dest, SPI_TRANSFER_TIMEOUT(len));
}

STATIC const mp_machine_spi_p_t machine_hard_spi_p = {
    .init = machine_hard_spi_init,
    .deinit = machine_hard_spi_deinit,
    .transfer = machine_hard_spi_transfer,
};

const mp_obj_type_t machine_hard_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .print = machine_hard_spi_print,
    .make_new = mp_machine_spi_make_new, // delegate to master constructor
    .protocol = &machine_hard_spi_p,
    .locals_dict = (mp_obj_dict_t*)&mp_machine_spi_locals_dict,
};

const spi_t *spi_from_mp_obj(mp_obj_t o) {
    if (MP_OBJ_IS_TYPE(o, &pyb_spi_type)) {
        pyb_spi_obj_t *self = MP_OBJ_TO_PTR(o);
        return self->spi;
    } else if (MP_OBJ_IS_TYPE(o, &machine_hard_spi_type)) {
        machine_hard_spi_obj_t *self = MP_OBJ_TO_PTR(o);
        return self->spi;
    } else {
        mp_raise_TypeError("expecting an SPI object");
    }
}
