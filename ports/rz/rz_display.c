/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
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
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"
// #include "dcache-control.h"
#include "mpy_file.h"
#include "mbed_jpeg.h"
#include "rz_buf.h"
#include "display.h"
#include "mbed_lcd.h"
#include "rz_display.h"

extern void dcache_clean(void *p_buf, uint32_t size);
extern void dcache_invalid(void *p_buf, uint32_t size);

// typedef unsigned int uint32_t;
// typedef unsigned char uint8_t;

STATIC mp_obj_t rz_display_jpeg_save(mp_obj_t self_in, mp_obj_t fn, mp_obj_t format) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char *filename = (char *)mp_obj_str_get_str(fn);
    int ret = display_jpeg_save_xy((const char *)filename, (const char *)self->dp.buf,
        self->dp.width, self->dp.height, self->dp.format);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(rz_display_jpeg_save_obj, rz_display_jpeg_save);

STATIC mp_obj_t rz_display_jpeg_load(mp_obj_t self_in, mp_obj_t fn, mp_obj_t format) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char *filename = (char *)mp_obj_str_get_str(fn);
    int ret = display_jpeg_load_xy((const char *)filename, (const char *)self->dp.buf,
        self->dp.width, self->dp.height, self->dp.format);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(rz_display_jpeg_load_obj, rz_display_jpeg_load);

STATIC mp_obj_t rz_display_get_fb_array(mp_obj_t self_in) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t size = (uint32_t)self->dp.stride * (uint32_t)self->dp.height;
    return mp_obj_new_bytearray_by_ref((size_t)size, (void *)self->dp.buf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_display_get_fb_array_obj, rz_display_get_fb_array);

STATIC mp_obj_t rz_display_get_fb_ptr(mp_obj_t self_in) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int((int)self->dp.buf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_display_get_fb_ptr_obj, rz_display_get_fb_ptr);

STATIC mp_obj_t rz_display_get_fb_size(mp_obj_t self_in) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t size = (uint32_t)self->dp.stride * (uint32_t)self->dp.height;
    return mp_obj_new_int(size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_display_get_fb_size_obj, rz_display_get_fb_size);

STATIC mp_obj_t rz_display_clear(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 1) {
        col = 0;
    } else if (n_args == 2) {
        col = (uint32_t)mp_obj_get_int(args[1]);
    } else if (n_args == 3) {
        col = (uint32_t)mp_obj_get_int(args[1]);
        self->dp.format = (uint16_t)mp_obj_get_int(args[2]);
    }
    display_clear(&self->dp, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_clear_obj, 1, 3, rz_display_clear);

STATIC mp_obj_t rz_display_pset(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 3) {
        col = self->dp.console.fcol;
    } else if (n_args == 4) {
        col = (uint32_t)mp_obj_get_int(args[3]);
    } else {
        col = (uint32_t)mp_obj_get_int(args[3]);
        self->dp.format = (uint16_t)mp_obj_get_int(args[4]);
    }
    uint16_t x = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[2]);
    display_pset(&self->dp, x, y, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_pset_obj, 3, 5, rz_display_pset);

STATIC mp_obj_t rz_display_box_sub(size_t n_args, const mp_obj_t *args, bool fill) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 5) {
        col = self->dp.console.fcol;
    } else if (n_args == 6) {
        col = (uint32_t)mp_obj_get_int(args[5]);
    } else {
        col = (uint32_t)mp_obj_get_int(args[5]);
        self->dp.format = (uint16_t)mp_obj_get_int(args[6]);
    }
    uint16_t x1 = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y1 = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t x2 = (uint16_t)mp_obj_get_int(args[3]);
    uint16_t y2 = (uint16_t)mp_obj_get_int(args[4]);
    if (fill) {
        display_box_fill(&self->dp, x1, y1, x2, y2, col);
    } else {
        display_box(&self->dp, x1, y1, x2, y2, col);
    }
    return mp_const_none;
}

STATIC mp_obj_t rz_display_box(size_t n_args, const mp_obj_t *args) {
    return rz_display_box_sub(n_args, args, false);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_box_obj, 5, 7, rz_display_box);

STATIC mp_obj_t rz_display_box_fill(size_t n_args, const mp_obj_t *args) {
    return rz_display_box_sub(n_args, args, true);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_box_fill_obj, 5, 7, rz_display_box_fill);

STATIC mp_obj_t rz_display_line(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 5) {
        col = self->dp.console.fcol;
    } else if (n_args == 6) {
        col = (uint32_t)mp_obj_get_int(args[5]);
    } else {
        col = (uint32_t)mp_obj_get_int(args[5]);
        self->dp.format = (uint16_t)mp_obj_get_int(args[6]);
    }
    uint16_t x1 = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y1 = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t x2 = (uint16_t)mp_obj_get_int(args[3]);
    uint16_t y2 = (uint16_t)mp_obj_get_int(args[4]);
    display_line(&self->dp, x1, y1, x2, y2, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_line_obj, 5, 7, rz_display_line);

STATIC mp_obj_t rz_display_circle_sub(size_t n_args, const mp_obj_t *args, bool fill) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 4) {
        col = self->dp.console.fcol;
    } else if (n_args == 5) {
        col = (uint32_t)mp_obj_get_int(args[4]);
    } else {
        col = (uint32_t)mp_obj_get_int(args[4]);
        self->dp.format = (uint16_t)mp_obj_get_int(args[5]);
    }
    uint16_t x = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[2]);
    uint16_t r = (uint16_t)mp_obj_get_int(args[3]);
    if (fill) {
        display_circle_fill(&self->dp, x, y, r, col);
    } else {
        display_circle(&self->dp, x, y, r, col);
    }
    return mp_const_none;
}

STATIC mp_obj_t rz_display_circle(size_t n_args, const mp_obj_t *args) {
    return rz_display_circle_sub(n_args, args, false);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_circle_obj, 4, 6, rz_display_circle);

STATIC mp_obj_t rz_display_circle_fill(size_t n_args, const mp_obj_t *args) {
    return rz_display_circle_sub(n_args, args, true);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_circle_fill_obj, 4, 6, rz_display_circle_fill);

STATIC mp_obj_t rz_display_fcol(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 1) {
        col = self->dp.console.fcol;
    } else {
        col = (uint32_t)mp_obj_get_int(args[1]);
        self->dp.console.fcol = col;
    }
    return mp_obj_new_int(col);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_fcol_obj, 1, 2, rz_display_fcol);

STATIC mp_obj_t rz_display_bcol(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t col = 0;
    if (n_args == 1) {
        col = self->dp.console.bcol;
    } else {
        col = (uint32_t)mp_obj_get_int(args[1]);
        self->dp.console.bcol = col;
    }
    return mp_obj_new_int(col);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_bcol_obj, 1, 2, rz_display_bcol);

STATIC mp_obj_t rz_display_font_id(mp_obj_t self_in, mp_obj_t idx) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_id = mp_obj_get_int(idx);
    font_t *font = get_font_by_id(font_id);
    if (font) {
        self->dp.console.font = font;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(rz_display_font_id_obj, rz_display_font_id);

STATIC mp_obj_t rz_display_putxy(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t x = (uint16_t)mp_obj_get_int(args[1]);
    uint16_t y = (uint16_t)mp_obj_get_int(args[2]);
    char *s = (char *)mp_obj_str_get_str(args[3]);
    display_write_char(&self->dp, *s, x, y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_putxy_obj, 4, 4, rz_display_putxy);

STATIC mp_obj_t rz_display_putc(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    char *s = (char *)mp_obj_str_get_str(args[1]);
    display_write_formatted_char(&self->dp, *s);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_putc_obj, 2, 2, rz_display_putc);

STATIC mp_obj_t rz_display_puts(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    char *s = (char *)mp_obj_str_get_str(args[1]);
    while (*s) {
        display_write_formatted_char(&self->dp, *s++);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_puts_obj, 2, 2, rz_display_puts);

STATIC mp_obj_t rz_display_pututf8(size_t n_args, const mp_obj_t *args) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t u;
    int len;
    char *s = (char *)mp_obj_str_get_str(args[1]);
    while (*s) {
        u = cnvUtf8ToUnicode((unsigned char *)s, (uint32_t *)&len);
        display_write_formatted_unicode(&self->dp, u);
        s += len;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_display_pututf8_obj, 2, 2, rz_display_pututf8);

STATIC mp_obj_t rz_display_start_display(mp_obj_t self_in) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mbed_lcd_start_display(&self->dp);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_display_start_display_obj, rz_display_start_display);

static qstr qstr_format(display_t *dp) {
    switch (dp->format) {
        case GFORMAT_YCBCR422:
            return MP_QSTR_G_YCBCR422;
        case GFORMAT_RGB565:
            return MP_QSTR_G_RGB565;
        case GFORMAT_RGB888:
            return MP_QSTR_G_RGB888;
        case GFORMAT_ARGB8888:
            return MP_QSTR_G_ARGB8888;
        case GFORMAT_ARGB4444:
            return MP_QSTR_G_ARGB4444;
        case GFORMAT_CLUT8:
            return MP_QSTR_G_CLUT8;
        case GFORMAT_CLUT4:
            return MP_QSTR_G_CLUT4;
        case GFORMAT_CLUT1:
            return MP_QSTR_G_CLUT1;
    }
    return MP_QSTR_G_YCBCR422;
}

static qstr qstr_swa(display_t *dp) {
    switch (dp->swa) {
        case WRSWA_NON:
            return MP_QSTR_WRSWA_NON;
        case WRSWA_8BIT:
            return MP_QSTR_WRSWA_8BIT;
        case WRSWA_16BIT:
            return MP_QSTR_WRSWA_16BIT;
        case WRSWA_16_8BIT:
            return MP_QSTR_WRSWA_16_8BIT;
        case WRSWA_32BIT:
            return MP_QSTR_WRSWA_32BIT;
        case WRSWA_32_8BIT:
            return MP_QSTR_WRSWA_32_8BIT;
        case WRSWA_32_16BIT:
            return MP_QSTR_WRSWA_32_16BIT;
        case WRSWA_32_16_8BIT:
            return MP_QSTR_WRSWA_32_16_8BIT;
    }
    return MP_QSTR_WRSWA_NON;
}

STATIC void rz_display_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    rz_display_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "DISPLAY layer_id=%d width=%d height=%d depth=%d stride=%d format=%q swa=%q, buf=%x hs=%d, vs=%d, hw=%d vw=%d",
        self->dp.layer_id, self->dp.width, self->dp.height, self->dp.depth, self->dp.stride, qstr_format(&self->dp), qstr_swa(&self->dp),
        self->dp.buf, self->dp.rect.hs, self->dp.rect.vs, self->dp.rect.hw, self->dp.rect.vw);
}

/// \classmethod \constructor(id)
/// Create a camera lcd object
///
STATIC mp_obj_t rz_display_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_font_id, ARG_width, ARG_height, ARG_format, ARG_swa, ARG_buf_ptr, ARG_stride, ARG_layer_id, ARG_hs, ARG_vs, ARG_hw, ARG_vw};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_font_id,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 4} },
        { MP_QSTR_width,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 640} },
        { MP_QSTR_height,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 480} },
        { MP_QSTR_format,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = GFORMAT_RGB565} },
        { MP_QSTR_swa,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = WRSWA_DEF} },
        { MP_QSTR_buf_ptr,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_stride,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_layer_id,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_hs,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_vs,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_hw,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_vw,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    rz_display_obj_t *self = m_new_obj(rz_display_obj_t);
    self->base.type = &rz_display_type;

    uint16_t layer_id = (uint16_t)args[ARG_layer_id].u_int;
    if (!((layer_id == 0) || (layer_id == 2) || (layer_id == 3))) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("layer_id(%d) not valid"), layer_id);
    }
    self->dp.width = (uint16_t)args[ARG_width].u_int;
    self->dp.height = (uint16_t)args[ARG_height].u_int;
    self->dp.console.cx = 0;
    self->dp.console.cy = 0;
    self->dp.console.fcol = White;
    self->dp.console.bcol = Black;
    self->dp.buf = (uint8_t *)args[ARG_buf_ptr].u_int;
    self->dp.format = (uint16_t)args[ARG_format].u_int;
    self->dp.swa = (uint16_t)args[ARG_swa].u_int;
    self->dp.stride = (uint16_t)args[ARG_stride].u_int;
    self->dp.layer_id = layer_id;
    self->dp.rect.hs = (uint16_t)args[ARG_hs].u_int;
    self->dp.rect.vs = (uint16_t)args[ARG_vs].u_int;
    self->dp.rect.hw = (uint16_t)args[ARG_hw].u_int;
    self->dp.rect.vw = (uint16_t)args[ARG_vw].u_int;
    /* set font */
    int font_id = args[ARG_font_id].u_int;
    int idx;
    if (!find_font_idx_by_id(font_id, &idx)) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("FONT(%d) doesn't exist"), font_id);
    }
    self->dp.console.font = get_font_by_id(font_id);
    self->dp.console.unit_wx = self->dp.console.font->fontUnitX;
    self->dp.console.unit_wy = self->dp.console.font->fontUnitY;
    display_init(&self->dp);
    switch (self->dp.format) {
        case GFORMAT_YCBCR422:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_32_16BIT;
            }
            break;
        case GFORMAT_RGB565:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_32_16BIT;
            }
            break;
        case GFORMAT_RGB888:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_NON;
            }
            break;
        case GFORMAT_ARGB8888:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_NON;
            }
            break;
        case GFORMAT_ARGB4444:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_NON;
            }
            break;
        case GFORMAT_CLUT8:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_32_16_8BIT;
            }
            break;
        case GFORMAT_CLUT4:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_32_16_8BIT;
            }
            break;
        case GFORMAT_CLUT1:
            if (self->dp.swa == WRSWA_DEF) {
                self->dp.swa = WRSWA_32_16_8BIT;
            }
            break;
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t rz_display_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_save_jpeg_file), MP_ROM_PTR(&rz_display_jpeg_save_obj) },
    { MP_ROM_QSTR(MP_QSTR_load_jpeg_file), MP_ROM_PTR(&rz_display_jpeg_load_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_display), MP_ROM_PTR(&rz_display_start_display_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_fb_array), MP_ROM_PTR(&rz_display_get_fb_array_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_fb_ptr), MP_ROM_PTR(&rz_display_get_fb_ptr_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_fb_size), MP_ROM_PTR(&rz_display_get_fb_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&rz_display_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_pset), MP_ROM_PTR(&rz_display_pset_obj) },
    { MP_ROM_QSTR(MP_QSTR_box), MP_ROM_PTR(&rz_display_box_obj) },
    { MP_ROM_QSTR(MP_QSTR_box_fill), MP_ROM_PTR(&rz_display_box_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&rz_display_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&rz_display_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle_fill), MP_ROM_PTR(&rz_display_circle_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_fcol), MP_ROM_PTR(&rz_display_fcol_obj) },
    { MP_ROM_QSTR(MP_QSTR_bcol), MP_ROM_PTR(&rz_display_bcol_obj) },
    { MP_ROM_QSTR(MP_QSTR_font_id), MP_ROM_PTR(&rz_display_font_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_putxy), MP_ROM_PTR(&rz_display_putxy_obj) },
    { MP_ROM_QSTR(MP_QSTR_putc), MP_ROM_PTR(&rz_display_putc_obj) },
    { MP_ROM_QSTR(MP_QSTR_puts), MP_ROM_PTR(&rz_display_puts_obj) },
    { MP_ROM_QSTR(MP_QSTR_pututf8), MP_ROM_PTR(&rz_display_pututf8_obj) },
    { MP_ROM_QSTR(MP_QSTR_G_YCBCR422), MP_ROM_INT(GFORMAT_YCBCR422) },
    { MP_ROM_QSTR(MP_QSTR_G_RGB565), MP_ROM_INT(GFORMAT_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_G_RGB888), MP_ROM_INT(GFORMAT_RGB888) },
    { MP_ROM_QSTR(MP_QSTR_G_ARGB8888), MP_ROM_INT(GFORMAT_ARGB8888) },
    { MP_ROM_QSTR(MP_QSTR_G_ARGB4444), MP_ROM_INT(GFORMAT_ARGB4444) },
    { MP_ROM_QSTR(MP_QSTR_G_CLUT8), MP_ROM_INT(GFORMAT_CLUT8) },
    { MP_ROM_QSTR(MP_QSTR_G_CLUT4), MP_ROM_INT(GFORMAT_CLUT4) },
    { MP_ROM_QSTR(MP_QSTR_G_CLUT1), MP_ROM_INT(GFORMAT_CLUT1) },
    { MP_ROM_QSTR(MP_QSTR_J_YCBCR422), MP_ROM_INT(JFORMAT_YCBCR422) },
    { MP_ROM_QSTR(MP_QSTR_J_ARGB8888), MP_ROM_INT(JFORMAT_ARGB8888) },
    { MP_ROM_QSTR(MP_QSTR_J_RGB565), MP_ROM_INT(JFORMAT_RGB565) },
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
    { MP_ROM_QSTR(MP_QSTR_Pink), MP_ROM_INT(Pink) }
};
STATIC MP_DEFINE_CONST_DICT(rz_display_locals_dict, rz_display_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rz_display_type,
    MP_QSTR_DISPLAY,
    MP_TYPE_FLAG_NONE,
    make_new, rz_display_obj_make_new,
    print, rz_display_obj_print,
    locals_dict, &rz_display_locals_dict
    );
