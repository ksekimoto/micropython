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

#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "font.h"

const mp_obj_type_t rz_font_type;

typedef struct _rz_font_obj_t {
    mp_obj_base_t base;
    uint32_t font_id;
    const font_t *font;
} rz_font_obj_t;

static const rz_font_obj_t rz_font_obj[] = {
    #ifdef MISAKIFONT4X8
    { {&rz_font_type}, MISAKIFONT4X8, (const font_t *)&MisakiFont4x8 },
    #endif
    #ifdef MISAKIFONT8X8
    { {&rz_font_type}, MISAKIFONT8X8, (const font_t *)&MisakiFont8x8 },
    #endif
    #ifdef MISAKIFONT6X12
    { {&rz_font_type}, MISAKIFONT6X12, (const font_t *)&MisakiFont6x12 },
    #endif
    #ifdef MISAKIFONT12X12
    { {&rz_font_type}, MISAKIFONT12X12, (const font_t *)&MisakiFont12x12 },
    #endif
};
#define NUM_FONTS (sizeof(rz_font_obj) / sizeof(rz_font_obj_t))

bool find_font_idx_by_id(int font_id, int *idx) {
    bool find = false;
    *idx = -1;
    for (uint32_t i = 0; i < NUM_FONTS; i++) {
        if (rz_font_obj[i].font_id == (uint32_t)font_id) {
            *idx = i;
            find = true;
            break;
        }
    }
    return find;
}

font_t *get_font_by_id(int font_id) {
    font_t *font = (font_t *)NULL;
    int idx;
    if (find_font_idx_by_id(font_id, &idx)) {
        font = (font_t *)rz_font_obj[idx].font;
    }
    return font;
}

/******************************************************************************/
/* MicroPython bindings                                                       */

void font_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    rz_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "FONT(%u):%s", self->font_id, self->font->fontName);
}

STATIC mp_obj_t rz_font_id(mp_obj_t self_in) {
    rz_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->font_id);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_font_id_obj, rz_font_id);

STATIC mp_obj_t rz_font_name(mp_obj_t self_in) {
    rz_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char *font_name = self->font->fontName;
    return mp_obj_new_str((const char *)font_name, strlen(font_name));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_font_name_obj, rz_font_name);

STATIC mp_obj_t rz_font_width(mp_obj_t self_in) {
    rz_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_width = self->font->fontWidth;
    return mp_obj_new_int(font_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_font_width_obj, rz_font_width);

STATIC mp_obj_t rz_font_height(mp_obj_t self_in) {
    rz_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_height = self->font->fontHeight;
    return mp_obj_new_int(font_height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_font_height_obj, rz_font_height);

STATIC mp_obj_t rz_font_data(mp_obj_t self_in, mp_obj_t idx) {
    rz_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_idx = mp_obj_get_int(idx);
    int font_bytes = font_fontBytes((font_t *)self->font, font_idx);
    unsigned char *font_data = font_fontData((font_t *)self->font, font_idx);
    return mp_obj_new_bytearray_by_ref((size_t)font_bytes, (void *)font_data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rz_font_data_obj, rz_font_data);

/// \classmethod \constructor(id)
/// Create an FONT object
///
STATIC mp_obj_t font_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    mp_int_t font_id = mp_obj_get_int(args[0]);
    // check font number
    int idx;
    if (!find_font_idx_by_id(font_id, &idx)) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("FONT(%d) doesn't exist"), font_id);
    }
    // return static font object
    const rz_font_obj_t *self = &rz_font_obj[idx];
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t font_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_font_id), MP_ROM_PTR(&rz_font_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&rz_font_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rz_font_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rz_font_height_obj) },
    #ifdef MISAKIFONT4X8
    { MP_ROM_QSTR(MP_QSTR_MISAKIA_8), MP_ROM_INT(MISAKIFONT4X8) },
    #endif
    #ifdef MISAKIFONT6X12
    { MP_ROM_QSTR(MP_QSTR_MISAKIA_12), MP_ROM_INT(MISAKIFONT6X12) },
    #endif
    #ifdef MISAKIFONT8X8
    { MP_ROM_QSTR(MP_QSTR_MISAKIU_8), MP_ROM_INT(MISAKIFONT8X8) },
    #endif
    #ifdef MISAKIFONT12X12
    { MP_ROM_QSTR(MP_QSTR_MISAKIU_12), MP_ROM_INT(MISAKIFONT12X12) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_fontdata), MP_ROM_PTR(&rz_font_data_obj) },
};

STATIC MP_DEFINE_CONST_DICT(font_locals_dict, font_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rz_font_type,
    MP_QSTR_FONT,
    MP_TYPE_FLAG_NONE,
    make_new, font_obj_make_new,
    locals_dict, &font_locals_dict
    );
