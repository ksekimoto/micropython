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
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "timer.h"
#include "dac.h"
//#include "dma.h"
#include "pin.h"

/// \moduleref pyb
/// \class DAC - digital to analog conversion
///
/// The DAC is used to output analog values (a specific voltage) on pin X5 or pin X6.
/// The voltage will be between 0 and 3.3V.
///
/// *This module will undergo changes to the API.*
///
/// Example usage:
///
///     from pyb import DAC
///
///     dac = DAC(1)            # create DAC 1 on pin X5
///     dac.write(128)          # write a value to the DAC (makes X5 1.65V)
///
/// To output a continuous sine-wave:
///
///     import math
///     from pyb import DAC
///
///     # create a buffer containing a sine-wave
///     buf = bytearray(100)
///     for i in range(len(buf)):
///         buf[i] = 128 + int(127 * math.sin(2 * math.pi * i / len(buf)))
///
///     # output the sine-wave at 400Hz
///     dac = DAC(1)
///     dac.write_timed(buf, 400 * len(buf), mode=DAC.CIRCULAR)

#if defined(MICROPY_HW_ENABLE_DAC) && MICROPY_HW_ENABLE_DAC

void dac_init(void) {
}

/******************************************************************************/
// MicroPython bindings

typedef struct _pyb_dac_obj_t {
    mp_obj_base_t base;
    uint32_t dac_channel;
    mp_hal_pin_obj_t pin;
} pyb_dac_obj_t;

STATIC void pyb_dac_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_dac_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "DAC(%u)",
        self->dac_channel == 0 ? 1 : 2);
}

STATIC mp_obj_t pyb_dac_init_helper(pyb_dac_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // GPIO configuration
    //mp_hal_pin_config(self->pin, MP_HAL_PIN_MODE_ANALOG, MP_HAL_PIN_PULL_NONE, 0);
    rx_dac_init((uint8_t)(self->pin->pin));
    return mp_const_none;
}

// create the dac object
// currently support either DAC1 on X5 (id = 1) or DAC2 on X6 (id = 2)

/// \classmethod \constructor(port)
/// Construct a new DAC object.
///
/// `port` can be a pin object, or an integer (1 or 2).
/// DAC(1) is on pin X5 and DAC(2) is on pin X6.
STATIC mp_obj_t pyb_dac_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get pin/channel to output on
    mp_int_t pin_idx;
    if (MP_OBJ_IS_INT(args[0])) {
        pin_idx = mp_obj_get_int(args[0]);
    } else {
        const pin_obj_t *pin = pin_find(args[0]);
        pin_idx = pin->pin;
        uint8_t channel = rx_dac_get_channel(pin_idx);
        if (channel == 0xff) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Pin(%q) doesn't have DAC capabilities", pin->name));
        }
    }
    pyb_dac_obj_t *dac = m_new_obj(pyb_dac_obj_t);
    dac->base.type = &pyb_dac_type;
    // configure the peripheral
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    pyb_dac_init_helper(dac, n_args - 1, args + 1, &kw_args);

    // return object
    return MP_OBJ_FROM_PTR(dac);
}

STATIC mp_obj_t pyb_dac_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_dac_init_helper(MP_OBJ_TO_PTR(args[0]), n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_dac_init_obj, 1, pyb_dac_init);

/// \method deinit()
/// Turn off the DAC, enable other use of pin.
STATIC mp_obj_t pyb_dac_deinit(mp_obj_t self_in) {
    //pyb_dac_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // ToDo: add dac_deinit()
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_dac_deinit_obj, pyb_dac_deinit);

/// \method write(value)
/// Direct access to the DAC output (8 bit only at the moment).
STATIC mp_obj_t pyb_dac_write(mp_obj_t self_in, mp_obj_t val) {
    pyb_dac_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rx_dac_write((uint8_t)(self->pin->pin), (uint16_t)mp_obj_get_int(val));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_dac_write_obj, pyb_dac_write);

STATIC const mp_rom_map_elem_t pyb_dac_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_dac_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pyb_dac_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&pyb_dac_write_obj) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_dac_locals_dict, pyb_dac_locals_dict_table);

const mp_obj_type_t pyb_dac_type = {
    { &mp_type_type },
    .name = MP_QSTR_DAC,
    .print = pyb_dac_print,
    .make_new = pyb_dac_make_new,
    .locals_dict = (mp_obj_dict_t*)&pyb_dac_locals_dict,
};

#endif // MICROPY_HW_ENABLE_DAC
