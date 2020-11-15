/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "ff.h"

#include "mbed_camera.h"
#include "camera.h"

typedef struct _pyb_camera_obj_t {
    mp_obj_base_t base;
    mp_uint_t camera_id;
    mp_uint_t camera_type;
} pyb_camera_obj_t;

#if MICROPY_PY_PYB_CAMERA_DV
STATIC const pyb_camera_obj_t camera_dv_obj = {
    {&pyb_camera_type},
    0,
    CAMERA_DV
};
#endif
#if MICROPY_PY_PYB_CAMERA_MIPI
STATIC const pyb_camera_obj_t camera_mipi_obj = {
    {&pyb_camera_type},
    0,
    CAMERA_MIPI
};
#endif

int pyb_jpeg_save_xy(const char *filename, const char *buf, uint32_t wx, uint32_t wy, uint32_t format) {
    FIL fp;
    FRESULT res;
    fs_user_mount_t *vfs_fat;
    const char *p_out;
    uint32_t size = wx * wy * 2;
    uint32_t writed;
    char *jpeg_buf;
    int err=0;

    mp_vfs_mount_t *vfs = mp_vfs_lookup_path(filename, &p_out);
    if (vfs != MP_VFS_NONE && vfs != MP_VFS_ROOT) {
        vfs_fat = MP_OBJ_TO_PTR(vfs->obj);
    } else {
#if defined(DEBUG_CAMERA)
        debug_printf("Cannot find user mount for %s\n", filename);
#endif
        return -1;
    }
    res = f_open(&vfs_fat->fatfs, &fp, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
#if defined(DEBUG_CAMERA)
        debug_printf("File can't be opened", filename);
#endif
        return -1;
    }
    writed = size;
    err = mbed_jpeg_encode(buf, wx, wy, (char **)&jpeg_buf, &writed, format);
    if ((err == 0) && (writed >= 0) && (writed < size)) {
        f_write(&fp, (void *)jpeg_buf, (UINT)writed, (UINT *)&writed);
    }
    f_close(&fp);
    return err;
}

int pyb_jpeg_load_xy(const char *filename, const char *buf, uint32_t wx, uint32_t wy, uint32_t format) {
    FIL fp;
    FRESULT res;
    fs_user_mount_t *vfs_fat;
    const char *p_out;
    uint32_t size = wx * wy * 2;
    uint32_t reads;
    char *jpeg_buf;
    int err=0;

    mp_vfs_mount_t *vfs = mp_vfs_lookup_path(filename, &p_out);
    if (vfs != MP_VFS_NONE && vfs != MP_VFS_ROOT) {
        vfs_fat = MP_OBJ_TO_PTR(vfs->obj);
    } else {
#if defined(DEBUG_CAMERA)
        debug_printf("Cannot find user mount for %s\n", filename);
#endif
        return -1;
    }
    res = f_open(&vfs_fat->fatfs, &fp, filename, FA_READ);
    if (res != FR_OK) {
#if defined(DEBUG_CAMERA)
        debug_printf("File can't be opened", filename);
#endif
        return -1;
    }
    reads = (uint32_t)f_size(&fp);
    jpeg_buf = (char *)malloc(reads);
    if (jpeg_buf) {
        if (reads > 0) {
            f_read(&fp, (void *)jpeg_buf, (UINT)reads, (UINT *)&reads);
        }
        err = mbed_jpeg_decode(buf, wx, wy, (char *)jpeg_buf, reads, format);
        free(jpeg_buf);
    }
    f_close(&fp);
    return err;
}

STATIC mp_obj_t pyb_jpeg_save(mp_obj_t self_in, mp_obj_t fn, mp_obj_t format) {
    char *filename = (char *)mp_obj_str_get_str(fn);
    uint32_t jformat = mp_obj_get_int(format);
    uint8_t *buf = mbed_get_camera_fb_ptr();
    uint32_t wx = (uint32_t)mbed_get_camera_hw();
    uint32_t wy = (uint32_t)mbed_get_camera_vw();
    int ret = pyb_jpeg_save_xy((const char *)filename, (const char *)buf, wx, wy, jformat);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_jpeg_save_obj, pyb_jpeg_save);

STATIC mp_obj_t pyb_jpeg_load(mp_obj_t self_in, mp_obj_t fn, mp_obj_t format) {
    char *filename = (char *)mp_obj_str_get_str(fn);
    uint32_t jformat = mp_obj_get_int(format);
    uint8_t *buf = mbed_get_camera_fb_ptr();
    uint32_t wx = (uint32_t)mbed_get_camera_hw();
    uint32_t wy = (uint32_t)mbed_get_camera_vw();
    int ret = pyb_jpeg_load_xy((const char *)filename, (const char *)buf, wx, wy, jformat);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_jpeg_load_obj, pyb_jpeg_load);

STATIC mp_obj_t pyb_get_frame_buffer(mp_obj_t self_in) {
    uint8_t *buf = mbed_get_lcd_fb_ptr();
    int hw = (int)mbed_get_lcd_hw();
    int vw = (int)mbed_get_lcd_vw();
    int pic_size = (int)mbed_get_lcd_pic_size();
    int size = (((hw * pic_size + 0x1f) & (~0x1f)) * vw);
    return mp_obj_new_bytearray_by_ref((size_t)size, (void *)buf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_get_frame_buffer_obj, pyb_get_frame_buffer);

STATIC mp_obj_t pyb_start_camera(mp_obj_t self_in, mp_obj_t format) {
    uint8_t *buf = mbed_get_camera_fb_ptr();
    uint32_t vformat = mp_obj_get_int(format);
    mbed_start_video_camera(buf, vformat);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_start_camera_obj, pyb_start_camera);

STATIC mp_obj_t pyb_start_lcd_display(mp_obj_t self_in, mp_obj_t format) {
    uint8_t *buf = mbed_get_camera_fb_ptr();
    uint32_t gformat = mp_obj_get_int(format);
    mbed_start_lcd_display(buf, gformat);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_start_lcd_display_obj, pyb_start_lcd_display);

STATIC void camera_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
}

/// \classmethod \constructor(id)
/// Create a camera lcd object
///
STATIC mp_obj_t camera_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_camera_type};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_type, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = CAMERA_DV} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    const pyb_camera_obj_t *self;
    if (args[0].u_int == CAMERA_DV) {
        self = (pyb_camera_obj_t *)&camera_dv_obj;
    } else {
        self = (pyb_camera_obj_t *)&camera_mipi_obj;
    }
    mbed_camera_init();
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t camera_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_GetFrameBuf), MP_ROM_PTR(&pyb_get_frame_buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_StartLcdDisplay), MP_ROM_PTR(&pyb_start_lcd_display_obj) },
    { MP_ROM_QSTR(MP_QSTR_StartCamera), MP_ROM_PTR(&pyb_start_camera_obj) },
    { MP_ROM_QSTR(MP_QSTR_JpegSave), MP_ROM_PTR(&pyb_jpeg_save_obj) },
    { MP_ROM_QSTR(MP_QSTR_JpegLoad), MP_ROM_PTR(&pyb_jpeg_load_obj) },
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
    { MP_ROM_QSTR(MP_QSTR_J_YCBCR422), MP_ROM_INT(JFORMAT_YCBCR422) },
    { MP_ROM_QSTR(MP_QSTR_J_ARGB8888), MP_ROM_INT(JFORMAT_ARGB8888) },
    { MP_ROM_QSTR(MP_QSTR_J_RGB565), MP_ROM_INT(JFORMAT_RGB565) },
};
STATIC MP_DEFINE_CONST_DICT(camera_locals_dict, camera_locals_dict_table);

const mp_obj_type_t pyb_camera_type = {
    { &mp_type_type },
    .name = MP_QSTR_CAMERA,
    .print = camera_obj_print,
    .make_new = camera_obj_make_new,
    .locals_dict = (mp_obj_dict_t*)&camera_locals_dict,
};
