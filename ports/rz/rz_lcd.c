/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

#include <stdint.h>
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "display.h"
#include "mbed_lcd.h"
#include "rz_lcd.h"

// typedef unsigned int uint32_t;
// typedef unsigned char uint8_t;

typedef struct _rz_lcd_obj_t {
    mp_obj_base_t base;
    lcd_t lcd;
} rz_lcd_obj_t;

static qstr qstr_lcd_name(lcd_t *lcd) {
    switch (lcd->lcd_id) {
        case GR_PEACH_4_3INCH_SHIELD:
            return MP_QSTR_GR_PEACH_4_3INCH_SHIELD;
        case GR_PEACH_7_1INCH_SHIELD:
            return MP_QSTR_GR_PEACH_7_1INCH_SHIELD;
        case GR_PEACH_DISPLAY_SHIELD:
            return MP_QSTR_GR_PEACH_DISPLAY_SHIELD;
        case RSK_TFT:
            return MP_QSTR_RSK_TFT;
        case TFP410PAP:
            return MP_QSTR_TFP410PAP;
        case TF043HV001A0:
            return MP_QSTR_TF043HV001A0;
        case ATM0430D25:
            return MP_QSTR_ATM0430D25;
        case FG040346DSSWBG03:
            return MP_QSTR_FG040346DSSWBG03;
        case LCD_800x480:
            return MP_QSTR_LCD_800x480;
        case RGB_TO_HDMI:
            return MP_QSTR_RGB_TO_HDMI;
    }
    return MP_QSTR_RGB_TO_HDMI;
}

STATIC void rz_lcd_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    rz_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "LCD(id=%x) name=%q width=%d height=%d",
        self->lcd.lcd_id, qstr_lcd_name(&self->lcd), self->lcd.width, self->lcd.height);
}

STATIC mp_obj_t rz_lcd_get_width(mp_obj_t self_in) {
    rz_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int((int)self->lcd.width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_lcd_get_width_obj, rz_lcd_get_width);

STATIC mp_obj_t rz_lcd_get_height(mp_obj_t self_in) {
    rz_lcd_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int((int)self->lcd.height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_lcd_get_height_obj, rz_lcd_get_height);

/// \classmethod \constructor(id)
/// Create a camera lcd object
///
STATIC mp_obj_t rz_lcd_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_lcd_id, };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_lcd_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    rz_lcd_obj_t *self = m_new_obj(rz_lcd_obj_t);
    self->base.type = &rz_lcd_type;
    self->lcd.lcd_id = (uint16_t)args[ARG_lcd_id].u_int;

    bool flag = mbed_lcd_init(&self->lcd);
    if (!flag) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("LCD init failed"));
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t rz_lcd_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rz_lcd_get_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rz_lcd_get_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_GR_PEACH_4_3INCH_SHIELD), MP_ROM_INT(GR_PEACH_4_3INCH_SHIELD) },
    { MP_ROM_QSTR(MP_QSTR_GR_PEACH_7_1INCH_SHIELD), MP_ROM_INT(GR_PEACH_7_1INCH_SHIELD) },
    { MP_ROM_QSTR(MP_QSTR_GR_PEACH_DISPLAY_SHIELD), MP_ROM_INT(GR_PEACH_DISPLAY_SHIELD) },
    { MP_ROM_QSTR(MP_QSTR_RSK_TFT), MP_ROM_INT(RSK_TFT) },
    { MP_ROM_QSTR(MP_QSTR_TFP410PAP), MP_ROM_INT(TFP410PAP) },
    { MP_ROM_QSTR(MP_QSTR_TF043HV001A0), MP_ROM_INT(TF043HV001A0) },
    { MP_ROM_QSTR(MP_QSTR_ATM0430D25), MP_ROM_INT(ATM0430D25) },
    { MP_ROM_QSTR(MP_QSTR_FG040346DSSWBG03), MP_ROM_INT(FG040346DSSWBG03) },
    { MP_ROM_QSTR(MP_QSTR_EP952), MP_ROM_INT(EP952) },
    { MP_ROM_QSTR(MP_QSTR_LCD_800x480), MP_ROM_INT(LCD_800x480) },
    { MP_ROM_QSTR(MP_QSTR_RGB_TO_HDMI), MP_ROM_INT(RGB_TO_HDMI) },
};
STATIC MP_DEFINE_CONST_DICT(rz_lcd_locals_dict, rz_lcd_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rz_lcd_type,
    MP_QSTR_LCD,
    MP_TYPE_FLAG_NONE,
    make_new, rz_lcd_obj_make_new,
    print, rz_lcd_obj_print,
    locals_dict, &rz_lcd_locals_dict
    );
