/*
 * Copyright (c) 2020, Kentaro Sekimoto
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

#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "extmod/machine_spi.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "font.h"
#include "jpeg.h"
#include "jpeg_disp.h"
#include "lcdspi.h"
// #include "common.h"
#include "pin.h"

#if MICROPY_HW_ENABLE_LCDSPI

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#define DEF_SPI_ID      0
#define DEF_BAUDRATE    115200

typedef struct _mod_lcdspi_obj_t {
    mp_obj_base_t base;
    mp_uint_t lcdspi_id;
    lcdspi_t *lcdspi;
    lcdspi_screen_t screen;
    lcdspi_pins_t pins;
} mod_lcdspi_obj_t;

static lcdspi_t m_lcdspi;

extern const mp_obj_type_t machine_hard_spi_type;
static mp_obj_t m_spi_obj;
static mp_machine_spi_p_t *machine_spi_p;
static mp_obj_t m_args[] = {
    MP_OBJ_NEW_SMALL_INT(1),
    MP_ROM_QSTR(MP_QSTR_baudrate),
    MP_OBJ_NEW_SMALL_INT(4000000),
};

static void lcdspi_spi_init_helper(void) {
    m_spi_obj = (mp_obj_t)machine_hard_spi_type.make_new(&machine_hard_spi_type, 1, 1, (const mp_obj_t *)m_args);
    machine_spi_p = (mp_machine_spi_p_t *)machine_hard_spi_type.protocol;
}

static void lcdspi_spi_transfer_helper(size_t len, const uint8_t *src, uint8_t *dest) {
    machine_spi_p->transfer((mp_obj_base_t *)m_spi_obj, (size_t)len, (const uint8_t *)src, (uint8_t *)dest);
}

/// \classmethod \constructor(id)
/// Create an LCDSPI object
///
STATIC mp_obj_t lcdspi_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_lcd_id, ARG_font_id, ARG_spi_id, ARG_baud, ARG_cs, ARG_clk, ARG_dout, ARG_reset, ARG_rs, ARG_din, ARG_polarity, ARG_phase, ARG_dir };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_lcd_id,   MP_ARG_REQUIRED | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_font_id,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_spi_id,   MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = -1} },
        { MP_QSTR_baud,     MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 4000000} },
        { MP_QSTR_cs,       MP_ARG_KW_ONLY | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_clk,      MP_ARG_KW_ONLY | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dout,     MP_ARG_KW_ONLY | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_reset,    MP_ARG_KW_ONLY | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rs,       MP_ARG_KW_ONLY | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_din,      MP_ARG_KW_ONLY | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 1} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_dir,      MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} }
    };
    // parse args
    mp_arg_val_t vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, vals);

    mod_lcdspi_obj_t *self = m_new_obj(mod_lcdspi_obj_t);
    self->base.type = type;
    self->screen.bcol = Black;
    self->screen.fcol = White;
    /* set lcdspi  */
    self->lcdspi_id = vals[ARG_lcd_id].u_int;
    self->lcdspi = &m_lcdspi;
    self->lcdspi->screen = &self->screen;
    self->lcdspi->pins = &self->pins;
    /* set font */
    int font_id = vals[ARG_font_id].u_int;
    int idx;
    if (!find_font_idx_by_id(font_id, &idx)) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("FONT(%d) doesn't exist"), font_id);
    }
    self->screen.font = get_font_by_id(font_id);
    /* set spi pins from input */
    /* baud */
    self->lcdspi->spi_ch = vals[ARG_spi_id].u_int;
    self->lcdspi->baud = vals[ARG_baud].u_int;
    self->lcdspi->polarity = vals[ARG_polarity].u_int;
    self->lcdspi->phase = vals[ARG_phase].u_int;
    /* cs */
    if (vals[ARG_cs].u_obj == MP_OBJ_NULL) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%q pin not specified"), allowed_args[ARG_cs].qst);
    } else if (!mp_obj_is_type(vals[ARG_cs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        pin_obj_t *pin = (pin_obj_t *)(vals[ARG_cs].u_obj);
        self->pins.pin_cs = (uint32_t)pin->id;
    }
    /* clk */
    if (vals[ARG_clk].u_obj == MP_OBJ_NULL) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%q pin not specified"), allowed_args[ARG_clk].qst);
    } else if (!mp_obj_is_type(vals[ARG_clk].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        pin_obj_t *pin = (pin_obj_t *)(vals[ARG_clk].u_obj);
        self->pins.pin_clk = (uint32_t)pin->id;
    }
    /* dout */
    if (vals[ARG_dout].u_obj == MP_OBJ_NULL) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%q pin not specified"), allowed_args[ARG_dout].qst);
    } else if (!mp_obj_is_type(vals[ARG_dout].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        pin_obj_t *pin = (pin_obj_t *)(vals[ARG_dout].u_obj);
        self->pins.pin_dout = (uint32_t)pin->id;
    }
    /* reset */
    if (vals[ARG_reset].u_obj == MP_OBJ_NULL) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%q pin not specified"), allowed_args[ARG_reset].qst);
    } else if (!mp_obj_is_type(vals[ARG_reset].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        pin_obj_t *pin = (pin_obj_t *)(vals[ARG_reset].u_obj);
        self->pins.pin_reset = (uint32_t)pin->id;
    }
    /* rs */
    if (vals[ARG_rs].u_obj == MP_OBJ_NULL) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("%q pin not specified"), allowed_args[ARG_rs].qst);
    } else if (!mp_obj_is_type(vals[ARG_rs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        pin_obj_t *pin = (pin_obj_t *)(vals[ARG_rs].u_obj);
        self->pins.pin_rs = (uint32_t)pin->id;
    }
    /* din */
    if (vals[ARG_din].u_obj == MP_OBJ_NULL) {
        // mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("%q pin not specified"), allowed_args[ARG_din].qst);
        self->pins.pin_din = PIN_NONE;
    } else if (!mp_obj_is_type(vals[ARG_din].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        pin_obj_t *pin = (pin_obj_t *)(vals[ARG_din].u_obj);
        self->pins.pin_din = pin->id;
    }
    int dir = vals[ARG_dir].u_int;

    m_args[0] = MP_OBJ_NEW_SMALL_INT(self->lcdspi->spi_ch);
    m_args[1] = MP_ROM_QSTR(MP_QSTR_baudrate);
    m_args[2] = MP_OBJ_NEW_SMALL_INT(self->lcdspi->baud);
    self->lcdspi->gpio_output = (lcdspi_gpio_output_t)rz_gpio_mode_output;
    self->lcdspi->gpio_input = (lcdspi_gpio_input_t)rz_gpio_mode_input;
    self->lcdspi->gpio_write = (lcdspi_gpio_write_t)rz_gpio_write;
    self->lcdspi->gpio_read = (lcdspi_gpio_read_t)rz_gpio_read;
    self->lcdspi->spi_init = (lcdspi_spi_init_t)lcdspi_spi_init_helper;
    self->lcdspi->spi_transfer = (lcdspi_spi_transfer_t)lcdspi_spi_transfer_helper;
    lcdspi_init(self->lcdspi, &self->screen, &self->pins, self->lcdspi_id, self->lcdspi->spi_ch);
    lcdspi_set_screen_dir(self->lcdspi, (uint8_t)dir);
    return MP_OBJ_FROM_PTR(self);
}

#if READ_LCD_ID
STATIC mp_obj_t mod_lcdspi_rddid(mp_obj_t self_in) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t val = self->lcdspi->did1;
    uint8_t did[8];
    did[0] = (uint8_t)((val >> 24) & 0xff);
    did[1] = (uint8_t)((val >> 16) & 0xff);
    did[2] = (uint8_t)((val >> 8) & 0xff);
    did[3] = (uint8_t)(val & 0xff);
    val = self->lcdspi->did2;
    did[4] = (uint8_t)((val >> 24) & 0xff);
    did[5] = (uint8_t)((val >> 16) & 0xff);
    did[6] = (uint8_t)((val >> 8) & 0xff);
    did[7] = (uint8_t)(val & 0xff);
    return mp_obj_new_bytearray(8, (void *)&did[0]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_lcdspi_rddid_obj, mod_lcdspi_rddid);

#define BYTE3(d)   (((uint32_t)(d) >> 24) & 0xff)
#define BYTE2(d)   (((uint32_t)(d) >> 16) & 0xff)
#define BYTE1(d)   (((uint32_t)(d) >> 8) & 0xff)
#define BYTE0(d)   (((uint32_t)(d) >> 0) & 0xff)

STATIC mp_obj_t mod_lcdspi_ili93xx_ids(mp_obj_t self_in) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t ids[8];
    ids[0] = BYTE3(self->lcdspi->ili93xx_id_spisw);
    ids[1] = BYTE2(self->lcdspi->ili93xx_id_spisw);
    ids[2] = BYTE1(self->lcdspi->ili93xx_id_spisw);
    ids[3] = BYTE0(self->lcdspi->ili93xx_id_spisw);
    ids[4] = BYTE3(self->lcdspi->ili93xx_id_spihw);
    ids[5] = BYTE2(self->lcdspi->ili93xx_id_spihw);
    ids[6] = BYTE1(self->lcdspi->ili93xx_id_spihw);
    ids[7] = BYTE0(self->lcdspi->ili93xx_id_spihw);
    return mp_obj_new_bytearray(sizeof(ids), (void *)&ids[0]);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mod_lcdspi_ili93xx_ids_obj, mod_lcdspi_ili93xx_ids);

#define LCDSPI_SPISW_READ_REG   0
#define LCDSPI_SPIHW_READ_REG   1
#define ILI93xx_SPISW_READ_REG  2
#define ILI93xx_SPIHW_READ_REG  3

STATIC mp_obj_t mod_lcdspi_read_reg(size_t n_args, const mp_obj_t *args, uint32_t id) {
    uint8_t addr = (uint8_t)mp_obj_get_int(args[1]);
    uint8_t idx = 0;
    uint8_t value = 0;
    if (n_args == 2) {
        idx = 0;
    } else {
        idx = (uint8_t)mp_obj_get_int(args[2]);
    }
    switch (id) {
        case LCDSPI_SPISW_READ_REG:
            value = lcdspi_spisw_read_reg(addr, idx);
            break;
        case LCDSPI_SPIHW_READ_REG:
            value = lcdspi_spihw_read_reg(addr, idx);
            break;
        case ILI93xx_SPISW_READ_REG:
            value = ILI93xx_spisw_read_reg(addr, idx);
            break;
        case ILI93xx_SPIHW_READ_REG:
            value = ILI93xx_spihw_read_reg(addr, idx);
            break;
    }
    return MP_OBJ_NEW_SMALL_INT(value);
}

STATIC mp_obj_t mod_lcdspi_spisw_read_reg(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_read_reg(n_args, args, LCDSPI_SPISW_READ_REG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_spisw_read_reg_obj, 2, 3, mod_lcdspi_spisw_read_reg);

STATIC mp_obj_t mod_lcdspi_spihw_read_reg(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_read_reg(n_args, args, LCDSPI_SPIHW_READ_REG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_spihw_read_reg_obj, 2, 3, mod_lcdspi_spihw_read_reg);

STATIC mp_obj_t mod_ili93xx_spisw_read_reg(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_read_reg(n_args, args, ILI93xx_SPISW_READ_REG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_ili93xx_spisw_read_reg_obj, 2, 3, mod_ili93xx_spisw_read_reg);

STATIC mp_obj_t mod_ili93xx_spihw_read_reg(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_read_reg(n_args, args, ILI93xx_SPIHW_READ_REG);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_ili93xx_spihw_read_reg_obj, 2, 3, mod_ili93xx_spihw_read_reg);
#endif
STATIC mp_obj_t mod_lcdspi_scroll(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t dy = (uint16_t)mp_obj_get_int(args[1]);
    lcdspi_scroll(self->lcdspi, dy);
    return mp_obj_new_int((int)dy);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_scroll_obj, 1, 2, mod_lcdspi_scroll);

STATIC mp_obj_t mod_lcdspi_clear(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 1) {
        col = 0;
    } else {
        col = (uint16_t)mp_obj_get_int(args[1]);
    }
    lcdspi_clear(self->lcdspi, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_clear_obj, 1, 2, mod_lcdspi_clear);

STATIC mp_obj_t mod_lcdspi_pset(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 3) {
        col = self->screen.fcol;
    } else {
        col = (uint16_t)mp_obj_get_int(args[3]);
    }
    uint16_t x = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[2]);
    lcdspi_pset(self->lcdspi, x, y, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_pset_obj, 3, 4, mod_lcdspi_pset);

STATIC mp_obj_t mod_lcdspi_box_sub(size_t n_args, const mp_obj_t *args, bool fill) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 5) {
        col = self->screen.fcol;
    } else {
        col = (uint16_t)mp_obj_get_int(args[5]);
    }
    uint32_t x1 = (uint32_t)mp_obj_get_int(args[1]);
    uint32_t y1 = (uint32_t)mp_obj_get_int(args[2]);
    uint32_t x2 = (uint32_t)mp_obj_get_int(args[3]);
    uint32_t y2 = (uint32_t)mp_obj_get_int(args[4]);
    if (fill) {
        lcdspi_box_fill(self->lcdspi, x1, y1, x2, y2, col);
    } else {
        lcdspi_box(self->lcdspi, x1, y1, x2, y2, col);
    }
    return mp_const_none;
}

STATIC mp_obj_t mod_lcdspi_box(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_box_sub(n_args, args, false);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_box_obj, 5, 6, mod_lcdspi_box);

STATIC mp_obj_t mod_lcdspi_box_fill(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_box_sub(n_args, args, true);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_box_fill_obj, 5, 6, mod_lcdspi_box_fill);

STATIC mp_obj_t mod_lcdspi_line(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 5) {
        col = self->screen.fcol;
    } else {
        col = (uint16_t)mp_obj_get_int(args[5]);
    }
    uint32_t x1 = (uint32_t)mp_obj_get_int(args[1]);
    uint32_t y1 = (uint32_t)mp_obj_get_int(args[2]);
    uint32_t x2 = (uint32_t)mp_obj_get_int(args[3]);
    uint32_t y2 = (uint32_t)mp_obj_get_int(args[4]);
    lcdspi_line(self->lcdspi, x1, y1, x2, y2, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_line_obj, 5, 6, mod_lcdspi_line);

STATIC mp_obj_t mod_lcdspi_circle_sub(size_t n_args, const mp_obj_t *args, bool fill) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 4) {
        col = self->screen.fcol;
    } else {
        col = (uint16_t)mp_obj_get_int(args[4]);
    }
    uint32_t x = (uint32_t)mp_obj_get_int(args[1]);
    uint32_t y = (uint32_t)mp_obj_get_int(args[2]);
    uint32_t r = (uint32_t)mp_obj_get_int(args[3]);
    if (fill) {
        lcdspi_circle_fill(self->lcdspi, x, y, r, col);
    } else {
        lcdspi_circle(self->lcdspi, x, y, r, col);
    }
    return mp_const_none;
}

STATIC mp_obj_t mod_lcdspi_circle(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_circle_sub(n_args, args, false);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_circle_obj, 4, 5, mod_lcdspi_circle);

STATIC mp_obj_t mod_lcdspi_circle_fill(size_t n_args, const mp_obj_t *args) {
    return mod_lcdspi_circle_sub(n_args, args, true);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_circle_fill_obj, 4, 5, mod_lcdspi_circle_fill);

STATIC mp_obj_t mod_lcdspi_fcol(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 1) {
        col = self->screen.fcol;
    } else {
        col = (uint16_t)mp_obj_get_int(args[1]);
        self->screen.fcol = col;
    }
    return mp_obj_new_int(col);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_fcol_obj, 1, 2, mod_lcdspi_fcol);

STATIC mp_obj_t mod_lcdspi_bcol(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t col = 0;
    if (n_args == 1) {
        col = self->screen.bcol;
    } else {
        col = (uint16_t)mp_obj_get_int(args[1]);
        self->screen.bcol = col;
    }
    return mp_obj_new_int(col);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_bcol_obj, 1, 2, mod_lcdspi_bcol);

STATIC mp_obj_t mod_lcdspi_font_id(mp_obj_t self_in, mp_obj_t idx) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_id = mp_obj_get_int(idx);
    font_t *font = get_font_by_id(font_id);
    if (font) {
        lcdspi_set_font(self->lcdspi, font);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_lcdspi_font_id_obj, mod_lcdspi_font_id);

STATIC mp_obj_t mod_lcdspi_putc_xy(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *s = (char *)mp_obj_str_get_str(args[3]);
    lcdspi_write_char_xy(self->lcdspi, *s, (uint32_t)x, (uint32_t)y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_putc_xy_obj, 4, 4, mod_lcdspi_putc_xy);

STATIC mp_obj_t mod_lcdspi_puts_xy(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *s = (char *)mp_obj_str_get_str(args[3]);
    while (*s) {
        lcdspi_write_char_xy(self->lcdspi, *s, (uint32_t)x, (uint32_t)y);
        s++;
        x += (int)font_fontUnitX(self->lcdspi->screen->font);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_puts_xy_obj, 4, 4, mod_lcdspi_puts_xy);

STATIC mp_obj_t mod_lcdspi_putc(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    char *s = (char *)mp_obj_str_get_str(args[1]);
    lcdspi_write_formatted_char(self->lcdspi, *s);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_putc_obj, 2, 2, mod_lcdspi_putc);

STATIC mp_obj_t mod_lcdspi_puts(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    char *s = (char *)mp_obj_str_get_str(args[1]);
    while (*s) {
        lcdspi_write_formatted_char(self->lcdspi, *s++);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_puts_obj, 2, 2, mod_lcdspi_puts);

STATIC mp_obj_t mod_lcdspi_pututf8_xy(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *s = (char *)mp_obj_str_get_str(args[3]);
    uint16_t u;
    int len;
    while (*s) {
        u = cnvUtf8ToUnicode((unsigned char *)s, (uint32_t *)&len);
        lcdspi_write_unicode_xy(self->lcdspi, u, (uint32_t)x, (uint32_t)y);
        s += len;
        if  (u >= 0x0100) {
            x += (int)font_fontUnitX(self->lcdspi->screen->font)*2;
        } else {
            x += (int)font_fontUnitX(self->lcdspi->screen->font);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_pututf8_xy_obj, 4, 4, mod_lcdspi_pututf8_xy);

STATIC mp_obj_t mod_lcdspi_pututf8(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t u;
    int len;
    char *s = (char *)mp_obj_str_get_str(args[1]);
    while (*s) {
        u = cnvUtf8ToUnicode((unsigned char *)s, (uint32_t *)&len);
        lcdspi_write_formatted_unicode(self->lcdspi, u);
        s += len;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_pututf8_obj, 2, 2, mod_lcdspi_pututf8);

STATIC mp_obj_t mod_lcdspi_bitblt(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int len;
    mp_obj_t *o;
    char *data;
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int wx = mp_obj_get_int(args[3]);
    int wy = mp_obj_get_int(args[4]);
    mp_obj_get_array(args[5], (size_t *)&len, &o);
    // ToDo: get byte array pointer
    data = (char *)o;
    lcdspi_bitbltex(self->lcdspi, x, y, wx, wy, (uint16_t *)data);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_bitblt_obj, 6, 6, mod_lcdspi_bitblt);

STATIC mp_obj_t mod_lcdspi_disp_bmp_file(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *fn = (char *)mp_obj_str_get_str(args[3]);
    lcdspi_disp_bmp_file(self->lcdspi, x, y, fn);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_disp_bmp_file_obj, 4, 4, mod_lcdspi_disp_bmp_file);

STATIC mp_obj_t mod_lcdspi_disp_jpeg_file(size_t n_args, const mp_obj_t *args) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *fn = (char *)mp_obj_str_get_str(args[3]);
    jpeg_disp_file(self->lcdspi, x, y, fn, true);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_lcdspi_disp_jpeg_file_obj, 4, 4, mod_lcdspi_disp_jpeg_file);

STATIC void lcdspi_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mod_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "LCDSPI(%u) name=%s width=%d height=%d\n",
        self->lcdspi_id, self->lcdspi->lcd->name,
        self->lcdspi->lcd->width, self->lcdspi->lcd->height);
}

STATIC const mp_rom_map_elem_t lcdspi_locals_dict_table[] = {
#if READ_LCD_ID
    { MP_ROM_QSTR(MP_QSTR_rddid), MP_ROM_PTR(&mod_lcdspi_rddid_obj) },
    { MP_ROM_QSTR(MP_QSTR_ili93xx_id), MP_ROM_PTR(&mod_lcdspi_ili93xx_ids_obj) },
    { MP_ROM_QSTR(MP_QSTR_spisw_reg), MP_ROM_PTR(&mod_lcdspi_spisw_read_reg_obj) },
    { MP_ROM_QSTR(MP_QSTR_reg), MP_ROM_PTR(&mod_lcdspi_spihw_read_reg_obj) },
    { MP_ROM_QSTR(MP_QSTR_ili93xx_spisw_reg), MP_ROM_PTR(&mod_ili93xx_spisw_read_reg_obj) },
    { MP_ROM_QSTR(MP_QSTR_ili93xx_reg), MP_ROM_PTR(&mod_ili93xx_spihw_read_reg_obj) },
#endif
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&mod_lcdspi_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_scroll), MP_ROM_PTR(&mod_lcdspi_scroll_obj) },
    { MP_ROM_QSTR(MP_QSTR_pset), MP_ROM_PTR(&mod_lcdspi_pset_obj) },
    { MP_ROM_QSTR(MP_QSTR_box), MP_ROM_PTR(&mod_lcdspi_box_obj) },
    { MP_ROM_QSTR(MP_QSTR_box_fill), MP_ROM_PTR(&mod_lcdspi_box_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&mod_lcdspi_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&mod_lcdspi_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle_fill), MP_ROM_PTR(&mod_lcdspi_circle_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_fcol), MP_ROM_PTR(&mod_lcdspi_fcol_obj) },
    { MP_ROM_QSTR(MP_QSTR_bcol), MP_ROM_PTR(&mod_lcdspi_bcol_obj) },
    { MP_ROM_QSTR(MP_QSTR_font_id), MP_ROM_PTR(&mod_lcdspi_font_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_putc), MP_ROM_PTR(&mod_lcdspi_putc_obj) },
    { MP_ROM_QSTR(MP_QSTR_putc_xy), MP_ROM_PTR(&mod_lcdspi_putc_xy_obj) },
    { MP_ROM_QSTR(MP_QSTR_puts), MP_ROM_PTR(&mod_lcdspi_puts_obj) },
    { MP_ROM_QSTR(MP_QSTR_puts_xy), MP_ROM_PTR(&mod_lcdspi_puts_xy_obj) },
    { MP_ROM_QSTR(MP_QSTR_pututf8), MP_ROM_PTR(&mod_lcdspi_pututf8_obj) },
    { MP_ROM_QSTR(MP_QSTR_pututf8_xy), MP_ROM_PTR(&mod_lcdspi_pututf8_xy_obj) },
    { MP_ROM_QSTR(MP_QSTR_bitblt), MP_ROM_PTR(&mod_lcdspi_bitblt_obj) },
    { MP_ROM_QSTR(MP_QSTR_disp_bmp_file), MP_ROM_PTR(&mod_lcdspi_disp_bmp_file_obj) },
    { MP_ROM_QSTR(MP_QSTR_disp_jpeg_file), MP_ROM_PTR(&mod_lcdspi_disp_jpeg_file_obj) },
    { MP_ROM_QSTR(MP_QSTR_C_PCF8833), MP_ROM_INT(PCF8833) },
    { MP_ROM_QSTR(MP_QSTR_C_S1D15G10), MP_ROM_INT(S1D15G10) },
    { MP_ROM_QSTR(MP_QSTR_C_ILI9341), MP_ROM_INT(ILI9341) },
    { MP_ROM_QSTR(MP_QSTR_C_ILI9340), MP_ROM_INT(ILI9340) },
    { MP_ROM_QSTR(MP_QSTR_C_ST7735), MP_ROM_INT(ST7735) },
    { MP_ROM_QSTR(MP_QSTR_C_ST7789), MP_ROM_INT(ST7789) },
    { MP_ROM_QSTR(MP_QSTR_M_NOKIA6100_0), MP_ROM_INT(NOKIA6100_0) },
    { MP_ROM_QSTR(MP_QSTR_M_NOKIA6100_1), MP_ROM_INT(NOKIA6100_1) },
    { MP_ROM_QSTR(MP_QSTR_M_T180), MP_ROM_INT(T180) },
    { MP_ROM_QSTR(MP_QSTR_M_M022C9340SPI), MP_ROM_INT(M022C9340SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_RASPI13LCDSPI), MP_ROM_INT(RASPI13LCDSPI) },
    { MP_ROM_QSTR(MP_QSTR_M_RASPI28LCDSPI), MP_ROM_INT(RASPI28LCDSPI) },
    { MP_ROM_QSTR(MP_QSTR_M_KMRTM24024SPI), MP_ROM_INT(KMRTM24024SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_KMR18SPI), MP_ROM_INT(KMR18SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_ST7735R_G128x160), MP_ROM_INT(ST7735R_G128x160) },
    { MP_ROM_QSTR(MP_QSTR_M_ST7735R_R128x160), MP_ROM_INT(ST7735R_R128x160) },
    { MP_ROM_QSTR(MP_QSTR_M_ST7735R_G128x128), MP_ROM_INT(ST7735R_G128x128) },
    { MP_ROM_QSTR(MP_QSTR_M_ST7735R_G160x80), MP_ROM_INT(ST7735R_G160x80) },
    { MP_ROM_QSTR(MP_QSTR_M_ROBOT_LCD), MP_ROM_INT(ROBOT_LCD) },
    { MP_ROM_QSTR(MP_QSTR_M_AIDEEPEN22SPI), MP_ROM_INT(AIDEEPEN22SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_PIM543), MP_ROM_INT(PIM543) },
    { MP_ROM_QSTR(MP_QSTR_M_WS_114SPI), MP_ROM_INT(WS_114SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_WS_13SPI), MP_ROM_INT(WS_13SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_WS_18SPI), MP_ROM_INT(WS_18SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_WS_28SPI), MP_ROM_INT(WS_28SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_WS_35SPI), MP_ROM_INT(WS_35SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_ST7735R_G130x161), MP_ROM_INT(ST7735R_G130x161) },
    { MP_ROM_QSTR(MP_QSTR_Black), MP_ROM_INT(Black) },
    { MP_ROM_QSTR(MP_QSTR_Navy), MP_ROM_INT(Navy) },
    { MP_ROM_QSTR(MP_QSTR_DarkGreen), MP_ROM_INT(DarkGreen) },
    { MP_ROM_QSTR(MP_QSTR_DarkCyan), MP_ROM_INT(DarkCyan) },
    { MP_ROM_QSTR(MP_QSTR_Maroon), MP_ROM_INT(Maroon) },
    { MP_ROM_QSTR(MP_QSTR_Purple), MP_ROM_INT(Purple) },
    { MP_ROM_QSTR(MP_QSTR_Olive), MP_ROM_INT(Olive) },
    { MP_ROM_QSTR(MP_QSTR_LightGrey), MP_ROM_INT(LightGrey) },
    { MP_ROM_QSTR(MP_QSTR_DarkGrey), MP_ROM_INT(DarkGrey) },
    { MP_ROM_QSTR(MP_QSTR_Blue), MP_ROM_INT(Blue) },
    { MP_ROM_QSTR(MP_QSTR_Green), MP_ROM_INT(Green) },
    { MP_ROM_QSTR(MP_QSTR_Cyan), MP_ROM_INT(Cyan) },
    { MP_ROM_QSTR(MP_QSTR_Red), MP_ROM_INT(Red) },
    { MP_ROM_QSTR(MP_QSTR_Magenta), MP_ROM_INT(Magenta) },
    { MP_ROM_QSTR(MP_QSTR_Yellow), MP_ROM_INT(Yellow) },
    { MP_ROM_QSTR(MP_QSTR_White), MP_ROM_INT(White) },
    { MP_ROM_QSTR(MP_QSTR_Orange), MP_ROM_INT(Orange) },
    { MP_ROM_QSTR(MP_QSTR_GreenYellow), MP_ROM_INT(GreenYellow) },
    { MP_ROM_QSTR(MP_QSTR_Pink), MP_ROM_INT(Pink) },
    { MP_ROM_QSTR(MP_QSTR_ROTATE_0), MP_ROM_INT(LCDSPI_ROTATE_0) },
    { MP_ROM_QSTR(MP_QSTR_ROTATE_90), MP_ROM_INT(LCDSPI_ROTATE_90) },
    { MP_ROM_QSTR(MP_QSTR_ROTATE_180), MP_ROM_INT(LCDSPI_ROTATE_180) },
    { MP_ROM_QSTR(MP_QSTR_ROTATE_270), MP_ROM_INT(LCDSPI_ROTATE_270) },
};
STATIC MP_DEFINE_CONST_DICT(lcdspi_locals_dict, lcdspi_locals_dict_table);

const mp_obj_type_t rz_lcdspi_type = {
    { &mp_type_type },
    .name = MP_QSTR_LCDSPI,
    .print = lcdspi_obj_print,
    .make_new = lcdspi_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&lcdspi_locals_dict,
};

#endif
