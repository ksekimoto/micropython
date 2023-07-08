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

#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "irq.h"
#include "pin.h"
#include "bufhelper.h"
#include "rx_i2c.h"
#include "i2c.h"

#if MICROPY_PY_PYB_LEGACY && MICROPY_HW_ENABLE_HW_I2C

#ifdef USE_DBG_PRINT
#include "debug_printf.h"
// #define DEBUG_I2C
#endif

// dummmy definition
// typedef int HAL_StatusTypeDef;
// #define HAL_OK  0
//

/// \moduleref pyb
/// \class I2C - a two-wire serial protocol
///
/// I2C is a two-wire protocol for communicating between devices.  At the physical
/// level it consists of 2 wires: SCL and SDA, the clock and data lines respectively.
///
/// I2C objects are created attached to a specific bus.  They can be initialised
/// when created, or initialised later on:
///
///     from pyb import I2C
///
///     i2c = I2C(1)                         # create on bus 1
///     i2c = I2C(1, I2C.CONTROLLER)         # create and init as a controller
///     i2c.init(I2C.CONTROLLER, baudrate=20000) # init as a controller
///     i2c.init(I2C.PERIPHERAL, addr=0x42)  # init as a peripheral with given address
///     i2c.deinit()                         # turn off the I2C unit
///
/// Printing the i2c object gives you information about its configuration.
///
/// Basic methods for peripheral are send and recv:
///
///     i2c.send('abc')      # send 3 bytes
///     i2c.send(0x42)       # send a single byte, given by the number
///     data = i2c.recv(3)   # receive 3 bytes
///
/// To receive inplace, first create a bytearray:
///
///     data = bytearray(3)  # create a buffer
///     i2c.recv(data)       # receive 3 bytes, writing them into data
///
/// You can specify a timeout (in ms):
///
///     i2c.send(b'123', timeout=2000)   # timeout after 2 seconds
///
/// A controller must specify the recipient's address:
///
///     i2c.init(I2C.CONTROLLER)
///     i2c.send('123', 0x42)        # send 3 bytes to peripheral with address 0x42
///     i2c.send(b'456', addr=0x42)  # keyword for address
///
/// Master also has other methods:
///
///     i2c.is_ready(0x42)           # check if peripheral 0x42 is ready
///     i2c.scan()                   # scan for peripherals on the bus, returning
///                                  #   a list of valid addresses
///     i2c.mem_read(3, 0x42, 2)     # read 3 bytes from memory of peripheral 0x42,
///                                  #   starting at address 2 in the peripheral
///     i2c.mem_write('abc', 0x42, 2, timeout=1000)
#define PYB_I2C_MASTER (0)
#define PYB_I2C_SLAVE  (1)
#define I2C_MEMADD_SIZE_8BIT    (0x00000001U)
#define I2C_MEMADD_SIZE_16BIT   (0x00000002U)

#define PYB_I2C_TIMEOUT         (1000)      // 1 second
#define PYB_I2C_SPEED_STANDARD (100000L)
#define PYB_I2C_SPEED_FULL     (400000L)
#define PYB_I2C_SPEED_FAST     (1000000L)

const pyb_i2c_obj_t pyb_i2c_obj[] = {
    #if defined(MICROPY_HW_I2C1_SCL)
    {{&pyb_i2c_type}, &i2c_obj[0]},
    #else
    {{&pyb_i2c_type}, NULL},
    #endif
    #if defined(MICROPY_HW_I2C2_SCL)
    {{&pyb_i2c_type}, &i2c_obj[1]},
    #else
    {{&pyb_i2c_type}, NULL},
    #endif
    #if defined(MICROPY_HW_I2C3_SCL)
    {{&pyb_i2c_type}, &i2c_obj[2]},
    #else
    {{&pyb_i2c_type}, NULL},
    #endif
    #if defined(MICROPY_HW_I2C4_SCL)
    {{&pyb_i2c_type}, &i2c_obj[3]},
    #else
    {{&pyb_i2c_type}, NULL},
    #endif
};

#define PYB_I2C_TIMINGR (0)

#define MICROPY_HW_I2C_BAUDRATE_DEFAULT (PYB_I2C_SPEED_FULL)
#define MICROPY_HW_I2C_BAUDRATE_MAX (PYB_I2C_SPEED_FULL)

STATIC void i2c_set_baudrate(i2c_t *i2c, uint32_t baudrate) {
    i2c->freq = baudrate;
    rx_i2c_set_baudrate(i2c->ch, baudrate);
}

uint32_t pyb_i2c_get_baudrate(i2c_t *i2c) {
    return i2c->freq;
}

void i2c_init0(void) {
}

int pyb_i2c_init(i2c_t *i2c) {
    // ToDo handling i2c_unit
    // int i2c_unit;
    const pin_obj_t *scl_pin;
    const pin_obj_t *sda_pin;

    if (0) {
    #if defined(MICROPY_HW_I2C1_SCL)
    } else if (i2c == &i2c_obj[0]) {
        // i2c_unit = 1;
        scl_pin = MICROPY_HW_I2C1_SCL;
        sda_pin = MICROPY_HW_I2C1_SDA;
    #endif
    #if defined(MICROPY_HW_I2C2_SCL)
    } else if (i2c == &i2c_obj[1]) {
        // i2c_unit = 2;
        scl_pin = MICROPY_HW_I2C2_SCL;
        sda_pin = MICROPY_HW_I2C2_SDA;
    #endif
    #if defined(MICROPY_HW_I2C3_SCL)
    } else if (i2c == &i2c_obj[2]) {
        // i2c_unit = 3;
        scl_pin = MICROPY_HW_I2C3_SCL;
        sda_pin = MICROPY_HW_I2C3_SDA;
    #endif
    #if defined(MICROPY_HW_I2C4_SCL)
    } else if (i2c == &i2c_obj[3]) {
        // i2c_unit = 4;
        scl_pin = MICROPY_HW_I2C4_SCL;
        sda_pin = MICROPY_HW_I2C4_SDA;
    #endif
    } else {
        // I2C does not exist for this board (shouldn't get here, should be checked by caller)
        return -MP_EINVAL;
    }
    // init the I2C module
    rx_i2c_init((uint32_t)i2c->ch, (uint32_t)scl_pin->id, (uint32_t)sda_pin->id, i2c->freq, PYB_I2C_TIMEOUT);
    return 0; // success
}

void i2c_deinit(i2c_t *i2c) {
    rx_i2c_deinit((uint32_t)i2c->ch);
}

void pyb_i2c_init_freq(const pyb_i2c_obj_t *self, mp_int_t freq) {
    // ToDo implement
    // baudrate configuration
    // init the I2C bus
    i2c_deinit(self->i2c);
    pyb_i2c_init(self->i2c);
}

/******************************************************************************/
/* MicroPython bindings                                                       */

static inline bool in_master_mode(pyb_i2c_obj_t *self) {
    return true;
}

STATIC void pyb_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    #if defined(MICROPY_HW_I2C1_SCL) | defined(MICROPY_HW_I2C3_SCL) | defined(MICROPY_HW_I2C3_SCL) | defined(MICROPY_HW_I2C4_SCL)
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    #endif

    uint i2c_num = 0;
    if (0) {
    }
    #if defined(MICROPY_HW_I2C1_SCL)
    else if (self->i2c == &i2c_obj[0]) {
        i2c_num = 1;
    }
    #endif
    #if defined(MICROPY_HW_I2C2_SCL)
    else if (self->i2c == &i2c_obj[1]) {
        i2c_num = 2;
    }
    #endif
    #if defined(MICROPY_HW_I2C3_SCL)
    else if (self->i2c == &i2c_obj[2]) {
        i2c_num = 3;
    }
    #endif
    #if defined(MICROPY_HW_I2C4_SCL)
    else if (self->i2c == &i2c_obj[3]) {
        i2c_num = 4;
    }
    #endif

    mp_printf(print, "I2C(%u)", i2c_num);
}

/// \method init(mode, *, addr=0x12, baudrate=400000, gencall=False)
///
/// Initialise the I2C bus with the given parameters:
///
///   - `mode` must be either `I2C.CONTROLLER` or `I2C.PERIPHERAL`
///   - `addr` is the 7-bit address (only sensible for a peripheral)
///   - `baudrate` is the SCL clock rate (only sensible for a controller)
///   - `gencall` is whether to support general call mode
STATIC mp_obj_t pyb_i2c_init_helper(const pyb_i2c_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = PYB_I2C_MASTER} },
        { MP_QSTR_addr,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0x12} },
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = MICROPY_HW_I2C_BAUDRATE_DEFAULT} },
        { MP_QSTR_gencall,  MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set the I2C configuration values
    if (args[0].u_int == PYB_I2C_SLAVE) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Slave is not supported"));
    }
    self->i2c->addr = args[1].u_int;
    // Set baudrate or timing value (if supported)
    i2c_set_baudrate(self->i2c, MIN(args[2].u_int, MICROPY_HW_I2C_BAUDRATE_MAX));
    // init the I2C bus
    i2c_deinit(self->i2c);
    int ret = pyb_i2c_init(self->i2c);
    if (ret != 0) {
        mp_raise_OSError(-ret);
    }

    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct an I2C object on the given bus.  `bus` can be 1 or 2.
/// With no additional parameters, the I2C object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the I2C buses are:
///
///   - `I2C(1)` is on the X position: `(SCL, SDA) = (X9, X10) = (PB6, PB7)`
///   - `I2C(2)` is on the Y position: `(SCL, SDA) = (Y9, Y10) = (PB10, PB11)`
STATIC mp_obj_t pyb_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get I2C object
    int i2c_id = i2c_find_peripheral(args[0]);
    const pyb_i2c_obj_t *i2c_obj = &pyb_i2c_obj[i2c_id - 1];

    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_i2c_init_helper(i2c_obj, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(i2c_obj);
}

STATIC mp_obj_t pyb_i2c_init_(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_i2c_init_helper(MP_OBJ_TO_PTR(args[0]), n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_init_obj, 1, pyb_i2c_init_);

/// \method deinit()
/// Turn off the I2C bus.
STATIC mp_obj_t pyb_i2c_deinit(mp_obj_t self_in) {
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    i2c_deinit(self->i2c);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_i2c_deinit_obj, pyb_i2c_deinit);

/// \method is_ready(addr)
/// Check if an I2C device responds to the given address.  Only valid when in controller mode.
STATIC mp_obj_t pyb_i2c_is_ready(mp_obj_t self_in, mp_obj_t i2c_addr_o) {
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (!in_master_mode(self)) {
        mp_raise_TypeError(MP_ERROR_TEXT("I2C must be a controller"));
    }

    mp_uint_t i2c_addr = mp_obj_get_int(i2c_addr_o);
    if (i2c_ready((i2c_t *)self->i2c, (uint16_t)i2c_addr)) {
        return mp_const_true;
    } else {
        return mp_const_false;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_i2c_is_ready_obj, pyb_i2c_is_ready);

/// \method scan()
/// Scan all I2C addresses from 0x08 to 0x77 and return a list of those that respond.
/// Only valid when in controller mode.
STATIC mp_obj_t pyb_i2c_scan(mp_obj_t self_in) {
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (!in_master_mode(self)) {
        mp_raise_TypeError(MP_ERROR_TEXT("I2C must be a controller"));
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);

    for (uint addr = 0x08; addr <= 0x77; addr++) {
        bool flag = i2c_ready((i2c_t *)self->i2c, (uint16_t)addr);
        if (flag) {
            mp_obj_list_append(list, MP_OBJ_NEW_SMALL_INT(addr));
        }
        #if defined(DEBUG_I2C)
        debug_printf("ADDR(%d):%d\r\n", addr, flag);
        #endif
    }

    return list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_i2c_scan_obj, pyb_i2c_scan);

/// \method send(send, addr=0x00, timeout=5000)
/// Send data on the bus:
///
///   - `send` is the data to send (an integer to send, or a buffer object)
///   - `addr` is the address to send to (only required in controller mode)
///   - `timeout` is the timeout in milliseconds to wait for the send
///
/// Return value: `None`.
STATIC mp_obj_t pyb_i2c_send(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_send,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_addr,    MP_ARG_INT, {.u_int = PYB_I2C_MASTER_ADDRESS} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get the buffer to send from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(args[0].u_obj, &bufinfo, data);

    // send the data
    xaction_t action;
    xaction_unit_t unit;
    rx_i2c_unit_init(&unit, (uint8_t *)bufinfo.buf, (uint32_t)bufinfo.len, false, (void *)NULL);
    rx_i2c_xaction_init(&action, (xaction_unit_t *)&unit, 1, self->i2c->ch, self->i2c->freq, (uint32_t)(args[1].u_int), true);
    rx_i2c_action_execute(&action, false, PYB_I2C_TIMEOUT);
    // i2c_reset_after_error(self->i2c);
    // mp_hal_raise(status);    // return the received data

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_send_obj, 1, pyb_i2c_send);

/// \method recv(recv, addr=0x00, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes
///   - `addr` is the address to receive from (only required in controller mode)
///   - `timeout` is the timeout in milliseconds to wait for the receive
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC mp_obj_t pyb_i2c_recv(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_recv,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_addr,    MP_ARG_INT, {.u_int = PYB_I2C_MASTER_ADDRESS} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get the buffer to receive into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(args[0].u_obj, &vstr);

    // receive the data
    xaction_t action;
    xaction_unit_t unit;
    rx_i2c_unit_init(&unit, (uint8_t *)vstr.buf, (uint32_t)vstr.len, true, (void *)NULL);
    rx_i2c_xaction_init(&action, (xaction_unit_t *)&unit, 1, self->i2c->ch, self->i2c->freq, (uint32_t)(args[1].u_int), true);
    rx_i2c_action_execute(&action, false, PYB_I2C_TIMEOUT);
    // i2c_reset_after_error(self->i2c);
    // mp_hal_raise(status);    // return the received data

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return o_ret;
    } else {
        return mp_obj_new_bytes_from_vstr(&vstr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_recv_obj, 1, pyb_i2c_recv);

/// \method mem_read(data, addr, memaddr, timeout=5000, addr_size=8)
///
/// Read from the memory of an I2C device:
///
///   - `data` can be an integer or a buffer to read into
///   - `addr` is the I2C device address
///   - `memaddr` is the memory location within the I2C device
///   - `timeout` is the timeout in milliseconds to wait for the read
///   - `addr_size` selects width of memaddr: 8 or 16 bits
///
/// Returns the read data.
/// This is only valid in controller mode.
STATIC const mp_arg_t pyb_i2c_mem_read_allowed_args[] = {
    { MP_QSTR_data,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_addr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_memaddr, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    { MP_QSTR_addr_size, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
};

STATIC mp_obj_t pyb_i2c_mem_read(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(pyb_i2c_mem_read_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(pyb_i2c_mem_read_allowed_args), pyb_i2c_mem_read_allowed_args, args);

    if (!in_master_mode(self)) {
        mp_raise_TypeError(MP_ERROR_TEXT("I2C must be a controller"));
    }

    // get the buffer to read into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(args[0].u_obj, &vstr);

    // get the addresses
    mp_uint_t i2c_addr = args[1].u_int << 1;
    mp_uint_t mem_addr = args[2].u_int;
    // determine width of mem_addr; default is 8 bits, entering any other value gives 16 bit width
    mp_uint_t mem_addr_size = I2C_MEMADD_SIZE_8BIT;
    if (args[4].u_int != 8) {
        mem_addr_size = I2C_MEMADD_SIZE_16BIT;
    }

    xaction_t action[2];
    xaction_unit_t units[2];
    uint8_t addrs[2];
    if (mem_addr_size == I2C_MEMADD_SIZE_8BIT) {
        addrs[0] = (uint8_t)mem_addr;
        rx_i2c_unit_init(&units[0], (uint8_t *)&addrs, 1, false, (void *)NULL);
    } else {
        addrs[0] = (uint8_t)(mem_addr >> 8);
        addrs[1] = (uint8_t)mem_addr;
        rx_i2c_unit_init(&units[0], (uint8_t *)&addrs, 2, false, (void *)NULL);
    }
    #if 0
    rx_i2c_unit_init(&units[1], (uint8_t *)vstr.buf, (uint32_t)vstr.len, true, (void *)NULL);
    rx_i2c_xaction_init(&action[0], (xaction_unit_t *)&units[0], 2, self->i2c->ch, self->i2c->freq, (uint32_t)(i2c_addr), true);
    rx_i2c_action_execute(&action[0], false, PYB_I2C_TIMEOUT);
    #else
    rx_i2c_unit_init(&units[1], (uint8_t *)vstr.buf, (uint32_t)vstr.len, true, (void *)NULL);
    rx_i2c_xaction_init(&action[0], (xaction_unit_t *)&units[0], 1, self->i2c->ch, self->i2c->freq, (uint32_t)(i2c_addr), false);
    rx_i2c_xaction_init(&action[1], (xaction_unit_t *)&units[1], 1, self->i2c->ch, self->i2c->freq, (uint32_t)(i2c_addr), true);
    rx_i2c_action_execute(&action[0], false, PYB_I2C_TIMEOUT);
    rx_i2c_action_execute(&action[1], true, PYB_I2C_TIMEOUT);
    #endif
    // return the read data
    if (o_ret != MP_OBJ_NULL) {
        return o_ret;
    } else {
        return mp_obj_new_bytes_from_vstr(&vstr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_mem_read_obj, 1, pyb_i2c_mem_read);

/// \method mem_write(data, addr, memaddr, timeout=5000, addr_size=8)
///
/// Write to the memory of an I2C device:
///
///   - `data` can be an integer or a buffer to write from
///   - `addr` is the I2C device address
///   - `memaddr` is the memory location within the I2C device
///   - `timeout` is the timeout in milliseconds to wait for the write
///   - `addr_size` selects width of memaddr: 8 or 16 bits
///
/// Returns `None`.
/// This is only valid in controller mode.
STATIC mp_obj_t pyb_i2c_mem_write(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args (same as mem_read)
    pyb_i2c_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(pyb_i2c_mem_read_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(pyb_i2c_mem_read_allowed_args), pyb_i2c_mem_read_allowed_args, args);

    if (!in_master_mode(self)) {
        mp_raise_TypeError(MP_ERROR_TEXT("I2C must be a controller"));
    }

    // get the buffer to write from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(args[0].u_obj, &bufinfo, data);

    // get the addresses
    mp_uint_t i2c_addr = args[1].u_int << 1;
    mp_uint_t mem_addr = args[2].u_int;
    // determine width of mem_addr; default is 8 bits, entering any other value gives 16 bit width
    mp_uint_t mem_addr_size = I2C_MEMADD_SIZE_8BIT;
    if (args[4].u_int != 8) {
        mem_addr_size = I2C_MEMADD_SIZE_16BIT;
    }

    xaction_t action;
    xaction_unit_t units[2];
    uint8_t addrs[2];
    if (mem_addr_size == I2C_MEMADD_SIZE_8BIT) {
        addrs[0] = (uint8_t)mem_addr;
        rx_i2c_unit_init(&units[0], (uint8_t *)&addrs, 1, false, (void *)&units[1]);
    } else {
        addrs[0] = (uint8_t)(mem_addr >> 8);
        addrs[1] = (uint8_t)mem_addr;
        rx_i2c_unit_init(&units[0], (uint8_t *)&addrs, 2, false, (void *)&units[1]);
    }
    rx_i2c_unit_init(&units[1], (uint8_t *)bufinfo.buf, (uint32_t)bufinfo.len, false, (void *)NULL);
    rx_i2c_xaction_init(&action, (xaction_unit_t *)&units[0], 2, self->i2c->ch, self->i2c->freq, (uint32_t)(i2c_addr), true);
    rx_i2c_action_execute(&action, false, PYB_I2C_TIMEOUT);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_mem_write_obj, 1, pyb_i2c_mem_write);

STATIC const mp_rom_map_elem_t pyb_i2c_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pyb_i2c_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_ready), MP_ROM_PTR(&pyb_i2c_is_ready_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&pyb_i2c_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&pyb_i2c_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&pyb_i2c_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem_read), MP_ROM_PTR(&pyb_i2c_mem_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem_write), MP_ROM_PTR(&pyb_i2c_mem_write_obj) },

    // class constants
    /// \constant CONTROLLER - for initialising the bus to controller mode
    /// \constant PERIPHERAL - for initialising the bus to peripheral mode
    { MP_ROM_QSTR(MP_QSTR_CONTROLLER), MP_ROM_INT(PYB_I2C_MASTER) },
    { MP_ROM_QSTR(MP_QSTR_PERIPHERAL),  MP_ROM_INT(PYB_I2C_SLAVE) },
    // TODO - remove MASTER/SLAVE when CONTROLLER/PERIPHERAL gain wide adoption
    { MP_ROM_QSTR(MP_QSTR_MASTER), MP_ROM_INT(PYB_I2C_MASTER) },
    #if 0
    // SLAVE is not implemented. Then the followings are not defined.
    { MP_ROM_QSTR(MP_QSTR_SLAVE), MP_ROM_INT(PYB_I2C_SLAVE) },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(pyb_i2c_locals_dict, pyb_i2c_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    pyb_i2c_type,
    MP_QSTR_I2C,
    MP_TYPE_FLAG_NONE,
    make_new, pyb_i2c_make_new,
    print, pyb_i2c_print,
    locals_dict, &pyb_i2c_locals_dict
    );

#endif // MICROPY_PY_PYB_LEGACY && MICROPY_HW_ENABLE_HW_I2C
