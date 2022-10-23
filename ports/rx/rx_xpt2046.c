/*
 * Copyright (c) 2021, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <stdint.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "extmod/machine_spi.h"
#include "spi.h"
#include "xpt2046.h"
#include "common.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "extmod/machine_spi.h"

#define DEF_SPI_ID  0
#define DEF_BAUDRATE 2000000

static mp_obj_t m_spi_obj;
static mp_machine_spi_p_t *machine_spi_p;

static pin_obj_t pin_mosi = {{&pin_type}, 0};
static pin_obj_t pin_miso = {{&pin_type}, 0};
static pin_obj_t pin_sck = {{&pin_type}, 0};

static mp_obj_t m_args[] = {
    MP_OBJ_NEW_SMALL_INT(DEF_SPI_ID),
    MP_ROM_QSTR(MP_QSTR_baudrate),
    MP_OBJ_NEW_SMALL_INT(DEF_BAUDRATE),
    MP_ROM_QSTR(MP_QSTR_mosi),
    MP_OBJ_FROM_PTR(&pin_mosi),
    MP_ROM_QSTR(MP_QSTR_miso),
    MP_OBJ_FROM_PTR(&pin_miso),
    MP_ROM_QSTR(MP_QSTR_sck),
    MP_OBJ_FROM_PTR(&pin_sck),
};

static void xpt2046_spi_init_helper(void) {
    m_spi_obj = MP_OBJ_TYPE_GET_SLOT(&machine_spi_type, make_new)(&machine_spi_type, 1, 4, (const mp_obj_t *)m_args);
    machine_spi_p = (mp_machine_spi_p_t *)MP_OBJ_TYPE_GET_SLOT(&machine_spi_type, protocol);
}

static void xpt2046_spi_transfer_helper(size_t len, const uint8_t *src, uint8_t *dest) {
    machine_spi_p->transfer((mp_obj_base_t *)m_spi_obj, (size_t)len, (const uint8_t *)src, (uint8_t *)dest);
}

typedef struct mod_xpt2046_obj_t {
    mp_obj_base_t base;
    xpt2046_t *xpt2046;
} mod_xpt2046_obj_t;

STATIC mod_xpt2046_obj_t *g_xpt2046 = (mod_xpt2046_obj_t *)NULL;
STATIC xpt2046_t m_xpt2046;

STATIC mp_obj_t xpt2046_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_baudrate, ARG_spi_id, ARG_mode, ARG_mosi, ARG_miso, ARG_clk, ARG_cs, ARG_irq,
           ARG_x_min, ARG_y_min, ARG_x_max, ARG_y_max, ARG_x_inv, ARG_y_inv, ARG_xy_swap,
           ARG_x_res, ARG_y_res, ARG_normalize};

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = XPT2046_BAUDRATE}},
        { MP_QSTR_spi_id, MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_mode, MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_mosi, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_miso, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_clk, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_cs, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_irq, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_x_min, MP_ARG_INT, {.u_int = 320}},
        { MP_QSTR_y_min, MP_ARG_INT, {.u_int = 320}},
        { MP_QSTR_x_max, MP_ARG_INT, {.u_int = 3900}},
        { MP_QSTR_y_max, MP_ARG_INT, {.u_int = 3900}},
        { MP_QSTR_x_inv, MP_ARG_BOOL, {.u_obj = mp_const_true}},
        { MP_QSTR_y_inv, MP_ARG_BOOL, {.u_obj = mp_const_true}},
        { MP_QSTR_xy_swap, MP_ARG_BOOL, {.u_obj = mp_const_false}},
        { MP_QSTR_x_res, MP_ARG_INT, {.u_int = 240}},
        { MP_QSTR_y_res, MP_ARG_INT, {.u_int = 320}},
        { MP_QSTR_normalize, MP_ARG_BOOL, {.u_obj = mp_const_true}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mod_xpt2046_obj_t *self = m_new_obj(mod_xpt2046_obj_t);
    self->base.type = type;
    self->xpt2046 = (xpt2046_t *)&m_xpt2046;
    self->xpt2046->baudrate = args[ARG_baudrate].u_int;
    self->xpt2046->spi_id = args[ARG_spi_id].u_int;
    self->xpt2046->mode = args[ARG_mode].u_int;
    if (args[ARG_mosi].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("mosi pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_mosi].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->xpt2046->mosi_id = ((pin_obj_t *)args[ARG_mosi].u_obj)->id;
    }
    if (args[ARG_miso].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("miso pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_miso].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->xpt2046->miso_id = ((pin_obj_t *)args[ARG_miso].u_obj)->id;
    }
    if (args[ARG_clk].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("clk pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_clk].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->xpt2046->clk_id = ((pin_obj_t *)args[ARG_clk].u_obj)->id;
    }
    if (args[ARG_cs].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("cs pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_cs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->xpt2046->cs_id = ((pin_obj_t *)args[ARG_cs].u_obj)->id;
    }
    if (args[ARG_irq].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("irq pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_irq].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->xpt2046->irq_id = ((pin_obj_t *)args[ARG_irq].u_obj)->id;
    }
    self->xpt2046->x_min = (int16_t)args[ARG_x_min].u_int;
    self->xpt2046->y_min = (int16_t)args[ARG_y_min].u_int;
    self->xpt2046->x_max = (int16_t)args[ARG_x_max].u_int;
    self->xpt2046->y_max = (int16_t)args[ARG_y_max].u_int;
    self->xpt2046->x_inv = (bool)args[ARG_x_inv].u_bool;
    self->xpt2046->y_inv = (bool)args[ARG_y_inv].u_bool;
    self->xpt2046->xy_swap = (bool)args[ARG_xy_swap].u_bool;
    self->xpt2046->x_res = (int16_t)args[ARG_x_res].u_int;
    self->xpt2046->y_res = (int16_t)args[ARG_y_res].u_int;
    self->xpt2046->normalize = (bool)args[ARG_normalize].u_bool;

    pin_mosi.id = self->xpt2046->mosi_id;
    pin_miso.id = self->xpt2046->miso_id;
    pin_sck.id = self->xpt2046->clk_id;
    m_args[0] = MP_OBJ_NEW_SMALL_INT(self->xpt2046->spi_id);
    m_args[1] = MP_ROM_QSTR(MP_QSTR_baudrate);
    m_args[2] = MP_OBJ_NEW_SMALL_INT(self->xpt2046->baudrate);
    self->xpt2046->gpio_output = (xpt2046_gpio_output_t)rx_gpio_mode_output;
    self->xpt2046->gpio_input = (xpt2046_gpio_input_t)rx_gpio_mode_input;
    self->xpt2046->gpio_write = (xpt2046_gpio_write_t)rx_gpio_write;
    self->xpt2046->gpio_read = (xpt2046_gpio_read_t)rx_gpio_read;
    self->xpt2046->spi_init = (xpt2046_spi_init_t)xpt2046_spi_init_helper;
    self->xpt2046->spi_transfer = (xpt2046_spi_transfer_t)xpt2046_spi_transfer_helper;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t mp_xpt2046_init(mp_obj_t self_in) {
    mod_xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    xpt2046_init(self->xpt2046);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_init_xpt2046_obj, mp_xpt2046_init);

STATIC mp_obj_t mp_xpt2046_deinit(mp_obj_t self_in) {
    mod_xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    xpt2046_deinit(self->xpt2046);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_deinit_xpt2046_obj, mp_xpt2046_deinit);

STATIC mp_obj_t mp_xpt2046_activate(mp_obj_t self_in) {
    mod_xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    xpt2046_activate(self->xpt2046);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_activate_xpt2046_obj, mp_xpt2046_activate);

static mp_obj_t mp_xpt2046_read(mp_obj_t self_in) {
    mod_xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    xpt2046_data_t data;
    xpt2046_read(self->xpt2046, &data);
    mp_obj_t tuple[3] = {
        mp_obj_new_bool(data.valid),
        mp_obj_new_int(data.x),
        mp_obj_new_int(data.y),
    };
    return mp_obj_new_tuple(3, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_xpt2046_read_obj, mp_xpt2046_read);

STATIC void xpt2046_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mod_xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "xpt2046 spi_id=%d\n", self->xpt2046->spi_id);
}

STATIC const mp_rom_map_elem_t xpt2046_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_xpt2046_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_xpt2046_obj) },
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&mp_activate_xpt2046_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_xpt2046_read_obj) },
};
STATIC MP_DEFINE_CONST_DICT(xpt2046_locals_dict, xpt2046_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rx_xpt2046_type,
    MP_QSTR_XPT2046,
    MP_TYPE_FLAG_NONE,
    make_new, xpt2046_make_new,
    locals_dict, &xpt2046_locals_dict
    );
