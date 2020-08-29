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

#include "mbed_camera_lcd.h"
#include "camera_lcd.h"

int pyb_jpeg_save_xy(const char *filename, const char* buf, uint32_t wx, uint32_t wy) {
    FIL fp;
    FRESULT res;
    fs_user_mount_t *vfs_fat;
    const char *p_out;
    uint32_t size = wx * wy * 2;
    uint32_t writed;
    char *jpeg_buf;

    mp_vfs_mount_t *vfs = mp_vfs_lookup_path(filename, &p_out);
    if (vfs != MP_VFS_NONE && vfs != MP_VFS_ROOT) {
        vfs_fat = MP_OBJ_TO_PTR(vfs->obj);
    } else {
#if defined(DEBUG_LCDSPI)
        debug_printf("Cannot find user mount for %s\n", filename);
#endif
        return -1;
    }
    res = f_open(&vfs_fat->fatfs, &fp, filename, FA_READ);
    if (res != FR_OK) {
#if defined(DEBUG_LCDSPI)
        debug_printf("File can't be opened", filename);
#endif
        return -1;
    }
    jpeg_buf = (char *)malloc(size);
    if (jpeg_buf) {
        mbed_jpeg_encode(buf, wx, wy, jpeg_buf, &writed);
        f_write(&fp, (void *)buf, (UINT)size, (UINT *)&writed);
        free(jpeg_buf);
        f_close(&fp);
    } else {
        f_close(&fp);
        return -1;
    }
    return 1;
}

STATIC mp_obj_t pyb_jpeg_save(mp_obj_t fn) {
    char *filename = (char *)mp_obj_str_get_str(fn);
    uint8_t *buf = mbed_get_camera_fb_ptr();
    uint32_t wx = 640;
    uint32_t wy = 480;
    pyb_jpeg_save_xy((const char *)filename, (const char *)buf, wx, wy);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_jpeg_save_obj, pyb_jpeg_save);

STATIC mp_obj_t pyb_get_frame_buffer(void) {
    uint8_t *buf = mbed_get_lcd_fb_ptr();
    int hw = (int)mbed_get_lcd_hw();
    int vw = (int)mbed_get_lcd_vw();
    int pic_size = (int)mbed_get_lcd_pic_size();
    int size = (((hw * pic_size + 0x1f) & (~0x1f)) * vw);
    return mp_obj_new_bytearray_by_ref((size_t)size, (void *)buf);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_get_frame_buffer_obj, pyb_get_frame_buffer);

STATIC mp_obj_t pyb_start_camera(void) {
    uint8_t *buf = mbed_get_camera_fb_ptr();
    mbed_start_video_camera(buf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_start_camera_obj, pyb_start_camera);

STATIC mp_obj_t pyb_start_lcd_ycbcr(void) {
    uint8_t *buf = mbed_get_camera_fb_ptr();
    mbed_start_lcd_ycbcr_display(buf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_start_lcd_ycbcr_obj, pyb_start_lcd_ycbcr);

STATIC mp_obj_t pyb_start_lcd_rgb(void) {
    uint8_t *buf = mbed_get_lcd_fb_ptr();
    mbed_start_lcd_rgb_display(buf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyb_start_lcd_rgb_obj, pyb_start_lcd_rgb);

/// \classmethod \constructor(id)
/// Create a camera lcd object
///
STATIC mp_obj_t camera_lcd_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mbed_camera_lcd_init();
    //uint8_t *buf = mbed_get_lcd_fb_ptr();
    //mbed_start_lcd_rgb_display(buf);
    return mp_const_none;
}

STATIC const mp_rom_map_elem_t camera_lcd_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_GetFrameBuf), MP_ROM_PTR(&pyb_get_frame_buffer_obj) },
    { MP_ROM_QSTR(MP_QSTR_StartLcdRgb), MP_ROM_PTR(&pyb_start_lcd_rgb_obj) },
    { MP_ROM_QSTR(MP_QSTR_StartLcdYcbcr), MP_ROM_PTR(&pyb_start_lcd_ycbcr_obj) },
    { MP_ROM_QSTR(MP_QSTR_StartCamera), MP_ROM_PTR(&pyb_start_camera_obj) },
    { MP_ROM_QSTR(MP_QSTR_JpegSave), MP_ROM_PTR(&pyb_jpeg_save_obj) },
};

STATIC MP_DEFINE_CONST_DICT(camera_lcd_locals_dict, camera_lcd_locals_dict_table);

const mp_obj_type_t pyb_camera_lcd_type = {
    { &mp_type_type },
    .name = MP_QSTR_CAMERA_LCD,
//    .print = camera_lcd_obj_print,
    .make_new = camera_lcd_obj_make_new,
    .locals_dict = (mp_obj_dict_t*)&camera_lcd_locals_dict,
};
