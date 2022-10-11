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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "ff.h"
#include "display.h"
#include "mbed_lcd.h"
#include "mbed_camera.h"
#include "mbed_jpeg.h"
#include "rz_camera.h"

typedef struct _rz_camera_obj_t {
    mp_obj_base_t base;
    camera_t camera;
    qstr name;
    uint16_t product_id;
} rz_camera_obj_t;

STATIC mp_obj_t rz_camera_type1_reg_write(size_t n_args, const mp_obj_t *args) {
    // rz_camera_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint8_t a = (uint8_t)mp_obj_get_int(args[1]);
    uint8_t r = (uint8_t)mp_obj_get_int(args[2]);
    uint8_t v = (uint8_t)mp_obj_get_int(args[3]);
    int ret = (int)camera_type1_reg_write(a, r, v);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_camera_type1_reg_write_obj, 4, 4, rz_camera_type1_reg_write);

STATIC mp_obj_t rz_camera_type1_reg_read(mp_obj_t self_in, mp_obj_t addr, mp_obj_t reg) {
    // rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t a = (uint8_t)mp_obj_get_int(addr);
    uint8_t r = (uint8_t)mp_obj_get_int(reg);
    uint8_t v;
    int ret = (int)camera_type1_reg_read(a, r, &v);
    (void)ret;
    return mp_obj_new_int((int)v);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(rz_camera_type1_reg_read_obj, rz_camera_type1_reg_read);

STATIC mp_obj_t rz_camera_type1_reg_tbl_write(mp_obj_t self_in, mp_obj_t addr, mp_obj_t ba) {
    // rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t a;
    int len;
    mp_obj_t *o;
    uint8_t *p;
    mp_obj_get_array(ba, (size_t *)&len, &o);
    p = (uint8_t *)o;
    a = (uint8_t)mp_obj_get_int(addr);
    int ret = (int)camera_type1_reg_tbl_write(a, (const uint8_t *)p, (size_t)len);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(rz_camera_type1_reg_tbl_write_obj, rz_camera_type1_reg_tbl_write);

STATIC mp_obj_t rz_camera_type2_reg_write(size_t n_args, const mp_obj_t *args) {
    // rz_camera_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint8_t a = (uint8_t)mp_obj_get_int(args[1]);
    uint8_t r = (uint8_t)mp_obj_get_int(args[2]);
    uint8_t v = (uint8_t)mp_obj_get_int(args[3]);
    int ret = (int)camera_type2_reg_write(a, r, v);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_camera_type2_reg_write_obj, 4, 4, rz_camera_type2_reg_write);

STATIC mp_obj_t rz_camera_type2_reg_read(mp_obj_t self_in, mp_obj_t addr, mp_obj_t reg) {
    // rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t a = (uint8_t)mp_obj_get_int(addr);
    uint16_t r = (uint8_t)mp_obj_get_int(reg);
    uint8_t v;
    int ret = (int)camera_type2_reg_read(a, r, &v);
    (void)ret;
    return mp_obj_new_int((int)v);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(rz_camera_type2_reg_read_obj, rz_camera_type2_reg_read);

STATIC mp_obj_t rz_camera_type2_reg_tbl_write(mp_obj_t self_in, mp_obj_t addr, mp_obj_t ba) {
    // rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t a;
    int len;
    mp_obj_t *o;
    uint8_t *p;
    mp_obj_get_array(ba, (size_t *)&len, &o);
    p = (uint8_t *)o;
    a = (uint8_t)mp_obj_get_int(addr);
    int ret = (int)camera_type2_reg_tbl_write(a, (const uint8_t *)p, (size_t)len);
    return mp_obj_new_int(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(rz_camera_type2_reg_tbl_write_obj, rz_camera_type2_reg_tbl_write);

STATIC mp_obj_t rz_camera_get_fb_array(mp_obj_t self_in) {
    rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t size = (uint32_t)self->camera.stride * (uint32_t)self->camera.vw;
    return mp_obj_new_bytearray_by_ref((size_t)size, (void *)self->camera.buf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_camera_get_fb_array_obj, rz_camera_get_fb_array);

STATIC mp_obj_t rz_camera_get_fb_ptr(mp_obj_t self_in) {
    rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int((int)self->camera.buf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_camera_get_fb_ptr_obj, rz_camera_get_fb_ptr);

STATIC mp_obj_t rz_camera_get_fb_size(mp_obj_t self_in) {
    rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint32_t size = (uint32_t)self->camera.stride * (uint32_t)self->camera.vw;
    return mp_obj_new_int(size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rz_camera_get_fb_size_obj, rz_camera_get_fb_size);

STATIC mp_obj_t rz_camera_start(size_t n_args, const mp_obj_t *args) {
    rz_camera_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 2) {
        self->camera.cformat = (uint16_t)mp_obj_get_int(args[1]);
    } else if (n_args == 3) {
        self->camera.cformat = (uint16_t)mp_obj_get_int(args[1]);
        self->camera.swa = (uint16_t)mp_obj_get_int(args[2]);
    }
    mbed_start_video_camera(&self->camera);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_camera_start_obj, 1, 3, rz_camera_start);

static qstr qstr_cformat(camera_t *camera) {
    switch (camera->cformat) {
        case CFORMAT_RGB888:
            return MP_QSTR_C_RGB888;
        case CFORMAT_RGB666:
            return MP_QSTR_C_RGB666;
        case CFORMAT_RGB565:
            return MP_QSTR_C_RGB565;
        case CFORMAT_BT656:
            return MP_QSTR_C_BT656;
        case CFORMAT_BT601:
            return MP_QSTR_C_BT601;
        case CFORMAT_YCBCR422:
            return MP_QSTR_C_YCBCR422;
        case CFORMAT_YCBCR444:
            return MP_QSTR_C_YCBCR444;
        default:
            return MP_QSTR_C_NONE;
    }
}

static qstr qstr_vformat(camera_t *camera) {
    switch (camera->vformat) {
        case VFORMAT_YCBCR422:
            return MP_QSTR_V_YCBCR422;
        case VFORMAT_RGB565:
            return MP_QSTR_V_RGB565;
        case VFORMAT_RGB888:
            return MP_QSTR_V_RGB888;
        case VFORMAT_RAW8:
            return MP_QSTR_V_RAW8;
    }
    return MP_QSTR_WRSWA_NON;
}

static qstr qstr_swa(camera_t *camera) {
    switch (camera->swa) {
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

STATIC void rz_camera_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    rz_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "CAMERA(id=%x name=%q id=%x width=%d height=%d depth=%d cformat=%q vformat=%q swa=%q buf=%x input_ch=%d)",
        self->camera.camera_id, self->name, self->product_id, self->camera.hw, self->camera.vw, self->camera.depth,
        qstr_cformat(&self->camera), qstr_vformat(&self->camera), qstr_swa(&self->camera), self->camera.buf, self->camera.input_ch);
}

/// \classmethod \constructor(id)
/// Create a camera lcd object
///
STATIC mp_obj_t rz_camera_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_camera_id, ARG_format, ARG_swa, ARG_buf_ptr, ARG_stride, ARG_input_ch, ARG_reset_level};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_camera_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = CAMERA_RASPBERRY_PI} },
        { MP_QSTR_format, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = CFORMAT_BT601} },
        { MP_QSTR_swa, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = WRSWA_32_16BIT} },
        { MP_QSTR_buf_ptr,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_stride,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_input_ch,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_reset_level,  MP_ARG_KW_ONLY | MP_ARG_INT,   {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    rz_camera_obj_t *self = m_new_obj(rz_camera_obj_t);
    self->base.type = &rz_camera_type;
    self->camera.camera_id = (uint16_t)args[ARG_camera_id].u_int;
    self->camera.buf = (uint8_t *)args[ARG_buf_ptr].u_int;
    self->camera.cformat = (uint16_t)args[ARG_format].u_int;
    self->camera.swa = (uint16_t)args[ARG_swa].u_int;
    self->camera.stride = (uint16_t)args[ARG_stride].u_int;
    self->camera.input_ch = (uint16_t)args[ARG_input_ch].u_int;
    self->camera.reset_level = args[ARG_reset_level].u_int;
    switch (self->camera.cformat) {
        case CFORMAT_RGB888:
            self->camera.vformat = VFORMAT_RGB888;
            break;
        case CFORMAT_RGB666:
            self->camera.vformat = VFORMAT_RGB888;
            break;
        case CFORMAT_RGB565:
            self->camera.vformat = VFORMAT_RGB565;
            break;
        case CFORMAT_BT656:
            self->camera.vformat = VFORMAT_RGB565;
            break;
        case CFORMAT_BT601:
            self->camera.vformat = VFORMAT_YCBCR422;
            break;
        case CFORMAT_YCBCR422:
            self->camera.vformat = VFORMAT_YCBCR422;
            break;
        case CFORMAT_YCBCR444:
            self->camera.vformat = VFORMAT_YCBCR422;
            break;
        case CFORMAT_RAW8:
        default:
            self->camera.vformat = VFORMAT_RAW8;
            break;
    }
    switch (self->camera.vformat) {
        case VFORMAT_RGB565:
            if (self->camera.swa == WRSWA_DEF) {
                self->camera.swa = WRSWA_NON;
            }
            break;
        case VFORMAT_RGB888:
            if (self->camera.swa == WRSWA_DEF) {
                self->camera.swa = WRSWA_NON;
            }
            break;
        case VFORMAT_RAW8:
            if (self->camera.swa == WRSWA_DEF) {
                self->camera.swa = WRSWA_NON;
            }
            break;
        case VFORMAT_YCBCR422:
        default:
            if (self->camera.swa == WRSWA_DEF) {
                self->camera.swa = WRSWA_32_16BIT;
            }
            break;
    }
    self->product_id = 0;
    qstr name_qst = MP_QSTR_NULL;
    switch (self->camera.camera_id) {
        case CAMERA_CVBS:
            name_qst = MP_QSTR_CAMERA_CVBS;
            break;
        case CAMERA_MT9V111:
            name_qst = MP_QSTR_CAMERA_MT9V111;
            self->product_id = camera_get_ov_product_id(0xb8, self->camera.reset_level);
            mp_printf(&mp_sys_stdout_print, "For MT9V111, not tested.\n");
            break;
        case CAMERA_OV7725:
            name_qst = MP_QSTR_CAMERA_OV7725;
            self->product_id = camera_get_ov_product_id(0x42, self->camera.reset_level);
            self->camera.cformat = CFORMAT_BT601;
            self->camera.vformat = VFORMAT_YCBCR422;
            mp_printf(&mp_sys_stdout_print, "For OV7725, only BT601 for camera, YCBCR422 for video are supported.\n");
            break;
        case CAMERA_OV5642:
            name_qst = MP_QSTR_CAMERA_OV5642;
            self->product_id = camera_get_ov_product_id(0x78, self->camera.reset_level);
            mp_printf(&mp_sys_stdout_print, "For OV5642, not tested.\n");
            break;
        case CAMERA_OV2640:
            name_qst = MP_QSTR_CAMERA_OV2640;
            self->product_id = camera_get_ov_product_id(0x42, self->camera.reset_level);
            mp_printf(&mp_sys_stdout_print, "For OV2640, not tested.\n");
            break;
        case CAMERA_OV7670:
            name_qst = MP_QSTR_CAMERA_OV7670;
            self->product_id = camera_get_ov_product_id(0x42, self->camera.reset_level);
            mp_printf(&mp_sys_stdout_print, "For OV7670, not tested.\n");
            break;
        case CAMERA_WIRELESS_CAMERA:
            name_qst = MP_QSTR_CAMERA_WIRELESS_CAMERA;
            mp_printf(&mp_sys_stdout_print, "For WIRELESS CAMERA, not tested.\n");
            break;
        case CAMERA_RASPBERRY_PI:
            name_qst = MP_QSTR_CAMERA_RASPBERRY_PI;
            if (self->camera.vformat != VFORMAT_RAW8) {
                mp_printf(&mp_sys_stdout_print, "So far, Only RAW8 is supported for RASPI CAMERA\n");
                self->camera.vformat = VFORMAT_RAW8;
            }
            break;
        case CAMERA_RASPBERRY_PI_WIDE_ANGLE:
            name_qst = MP_QSTR_CAMERA_RASPBERRY_PI_WIDE_ANGLE;
            if (self->camera.vformat != VFORMAT_RAW8) {
                mp_printf(&mp_sys_stdout_print, "So far, Only RAW8 is supported for RASPI CAMERA\n");
                self->camera.vformat = VFORMAT_RAW8;
            }
            break;
        case CAMERA_RASPBERRY_PI_832X480:
            name_qst = MP_QSTR_CAMERA_RASPBERRY_PI_832X480;
            if (self->camera.vformat != VFORMAT_RAW8) {
                mp_printf(&mp_sys_stdout_print, "So far, Only RAW8 is supported for RASPI CAMERA\n");
                self->camera.vformat = VFORMAT_RAW8;
            }
            break;
    }
    self->name = name_qst;
    mbed_camera_init(&self->camera);
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t camera_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_camera_type1_reg_tbl_write), MP_ROM_PTR(&rz_camera_type1_reg_tbl_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_camera_type1_reg_write), MP_ROM_PTR(&rz_camera_type1_reg_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_camera_type1_reg_read), MP_ROM_PTR(&rz_camera_type1_reg_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_camera_type2_reg_tbl_write), MP_ROM_PTR(&rz_camera_type2_reg_tbl_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_camera_type2_reg_write), MP_ROM_PTR(&rz_camera_type2_reg_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_camera_type2_reg_read), MP_ROM_PTR(&rz_camera_type2_reg_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_fb_array), MP_ROM_PTR(&rz_camera_get_fb_array_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_fb_ptr), MP_ROM_PTR(&rz_camera_get_fb_ptr_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_fb_size), MP_ROM_PTR(&rz_camera_get_fb_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_camera), MP_ROM_PTR(&rz_camera_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_DV), MP_ROM_INT(CAMERA_DV) },
    { MP_ROM_QSTR(MP_QSTR_MIPI), MP_ROM_INT(CAMERA_MIPI) },
    { MP_ROM_QSTR(MP_QSTR_G_YCBCR422), MP_ROM_INT(GFORMAT_YCBCR422) },
    { MP_ROM_QSTR(MP_QSTR_G_RGB565), MP_ROM_INT(GFORMAT_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_G_RGB888), MP_ROM_INT(GFORMAT_RGB888) },
    { MP_ROM_QSTR(MP_QSTR_G_ARGB8888), MP_ROM_INT(GFORMAT_ARGB8888) },
    { MP_ROM_QSTR(MP_QSTR_G_ARGB4444), MP_ROM_INT(GFORMAT_ARGB4444) },
    { MP_ROM_QSTR(MP_QSTR_G_CLUT8), MP_ROM_INT(GFORMAT_CLUT8) },
    { MP_ROM_QSTR(MP_QSTR_G_CLUT4), MP_ROM_INT(GFORMAT_CLUT4) },
    { MP_ROM_QSTR(MP_QSTR_G_CLUT1), MP_ROM_INT(GFORMAT_CLUT1) },
    { MP_ROM_QSTR(MP_QSTR_V_YCBCR422), MP_ROM_INT(VFORMAT_YCBCR422) },
    { MP_ROM_QSTR(MP_QSTR_V_RGB565), MP_ROM_INT(VFORMAT_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_V_RGB888), MP_ROM_INT(VFORMAT_RGB888) },
    { MP_ROM_QSTR(MP_QSTR_V_RAW8), MP_ROM_INT(VFORMAT_RAW8) },
    { MP_ROM_QSTR(MP_QSTR_C_RGB888), MP_ROM_INT(CFORMAT_RGB888) },
    { MP_ROM_QSTR(MP_QSTR_C_RGB666), MP_ROM_INT(CFORMAT_RGB666) },
    { MP_ROM_QSTR(MP_QSTR_C_RGB565), MP_ROM_INT(CFORMAT_RGB565) },
    { MP_ROM_QSTR(MP_QSTR_C_BT656), MP_ROM_INT(CFORMAT_BT656) },
    { MP_ROM_QSTR(MP_QSTR_C_BT601), MP_ROM_INT(CFORMAT_BT601) },
    { MP_ROM_QSTR(MP_QSTR_C_YCBCR422), MP_ROM_INT(CFORMAT_YCBCR422) },
    { MP_ROM_QSTR(MP_QSTR_C_YCBCR444), MP_ROM_INT(CFORMAT_YCBCR444) },
    { MP_ROM_QSTR(MP_QSTR_C_RAW8), MP_ROM_INT(CFORMAT_RAW8) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_NON), MP_ROM_INT(WRSWA_NON) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_8BIT), MP_ROM_INT(WRSWA_8BIT) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_16BIT), MP_ROM_INT(WRSWA_16BIT) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_16_8BIT), MP_ROM_INT(WRSWA_16_8BIT) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_32BIT), MP_ROM_INT(WRSWA_32BIT) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_32_8BIT), MP_ROM_INT(WRSWA_32_8BIT) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_32_16BIT), MP_ROM_INT(WRSWA_32_16BIT) },
    { MP_ROM_QSTR(MP_QSTR_WRSWA_32_16_8BIT), MP_ROM_INT(WRSWA_32_16_8BIT) },
    { MP_ROM_QSTR(MP_QSTR_CVBS), MP_ROM_INT(CAMERA_CVBS) },
    { MP_ROM_QSTR(MP_QSTR_MT9V111), MP_ROM_INT(CAMERA_MT9V111) },
    { MP_ROM_QSTR(MP_QSTR_OV7725), MP_ROM_INT(CAMERA_OV7725) },
    { MP_ROM_QSTR(MP_QSTR_OV5642), MP_ROM_INT(CAMERA_OV5642) },
    { MP_ROM_QSTR(MP_QSTR_OV2640), MP_ROM_INT(CAMERA_OV2640) },
    { MP_ROM_QSTR(MP_QSTR_OV7670), MP_ROM_INT(CAMERA_OV7670) },
    { MP_ROM_QSTR(MP_QSTR_WIRELESS_CAMERA), MP_ROM_INT(CAMERA_WIRELESS_CAMERA) },
    { MP_ROM_QSTR(MP_QSTR_RASPBERRY_PI), MP_ROM_INT(CAMERA_RASPBERRY_PI) },
    { MP_ROM_QSTR(MP_QSTR_RASPBERRY_PI_WIDE_ANGLE), MP_ROM_INT(CAMERA_RASPBERRY_PI_WIDE_ANGLE) },
    { MP_ROM_QSTR(MP_QSTR_RASPBERRY_PI_832X480), MP_ROM_INT(CAMERA_RASPBERRY_PI_832X480) },
};
STATIC MP_DEFINE_CONST_DICT(camera_locals_dict, camera_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    rz_camera_type,
    MP_QSTR_CAMERA,
    MP_TYPE_FLAG_NONE,
    make_new, rz_camera_obj_make_new,
    print, rz_camera_obj_print,
    locals_dict, &camera_locals_dict
    );
