/*
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
#include <memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/binary.h"
#include "py/objarray.h"
#include "r_dk2_if.h"
#include "r_dk2_core.h"
#include "r_drp_simple_isp.h"
#include "rz_buf.h"
#include "rz_isp.h"
#include "rz_drp.h"

#if USE_DRP

// ==================================================================
// DRP class
// ==================================================================

typedef struct _rz_drp_obj_t {
    mp_obj_base_t base;
    drp_t drp;
    rz_isp_obj_t *isp;
} rz_drp_obj_t;

static void  cb_load(uint8_t id) {
    // ToDo: implement
}

static void  cb_int(uint8_t id) {
    // ToDo: implement
}

STATIC mp_obj_t rz_drp_dk2_isp(size_t n_args, const mp_obj_t *args) {
    rz_drp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args == 2) {
        self->isp = MP_OBJ_TO_PTR(args[1]);
        return mp_const_none;
    } else {
        return MP_OBJ_FROM_PTR(self->isp);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_drp_dk2_isp_obj, 1, 2, rz_drp_dk2_isp);

STATIC mp_obj_t rz_drp_dk2_start(size_t n_args, const mp_obj_t *args) {
    rz_drp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    R_DK2_Start((const uint8_t)self->drp.id, (const void *const)self->isp->param_isp, sizeof(r_drp_simple_isp_t));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_drp_dk2_start_obj, 8, 8, rz_drp_dk2_start);

STATIC mp_obj_t rz_drp_dk2_load(size_t n_args, const mp_obj_t *args) {
    rz_drp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    void *pconfig = (void *)MP_OBJ_TO_PTR(args[1]);
    uint8_t top_tiles = (uint8_t)mp_obj_get_int(args[2]);
    uint32_t tile_pattern = (uint32_t)mp_obj_get_int(args[3]);
    uint8_t id = 0;
    R_DK2_Load((const void *const)pconfig, top_tiles, tile_pattern, (const load_cb_t)&cb_load, (const int_cb_t)&cb_int, (uint8_t *const)&id);
    self->drp.id = id;
    R_DK2_Activate(0, 0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_drp_dk2_load_obj, 4, 4, rz_drp_dk2_load);

STATIC void rz_drp_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    // rz_drp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "DRP");
}

/// \classmethod \constructor(id)
/// Create a drp object
///
STATIC mp_obj_t rz_drp_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_drp_id, ARG_drp_isp};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_isp, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    rz_drp_obj_t *self = m_new_obj(rz_drp_obj_t);
    self->base.type = &rz_drp_type;
    self->drp.id = (uint16_t)args[ARG_drp_id].u_int;
    self->isp = args[ARG_drp_isp].u_obj;

    int32_t ret = R_DK2_Initialize();
    if (ret != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("DRP init failed"));
    }
    return MP_OBJ_FROM_PTR(self);
}

#define OBJ_BYTEARRY(PARAM)  {{&mp_type_bytearray}, .typecode = BYTEARRAY_TYPECODE, .free = 0, .len = sizeof(PARAM), .items = (void *)&(PARAM) }

STATIC const mp_obj_array_t simple_isp_bayer2grayscale_3_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2grayscale_3);
STATIC const mp_obj_array_t simple_isp_bayer2grayscale_6_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2grayscale_6);
STATIC const mp_obj_array_t simple_isp_bayer2rgb_6_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2rgb_6);
STATIC const mp_obj_array_t simple_isp_bayer2yuv_3_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2yuv_3);
STATIC const mp_obj_array_t simple_isp_bayer2yuv_6_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2yuv_6);
STATIC const mp_obj_array_t simple_isp_bayer2yuv_planar_3_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2yuv_planar_3);
STATIC const mp_obj_array_t simple_isp_bayer2yuv_planar_6_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_bayer2yuv_planar_6);
STATIC const mp_obj_array_t simple_isp_grayscale_3_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_grayscale_3);
STATIC const mp_obj_array_t simple_isp_grayscale_6_obj = OBJ_BYTEARRY(g_drp_lib_simple_isp_grayscale_6);

STATIC const mp_rom_map_elem_t rz_drp_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_dk2_isp), MP_ROM_PTR(&rz_drp_dk2_isp_obj) },
    { MP_ROM_QSTR(MP_QSTR_dk2_start), MP_ROM_PTR(&rz_drp_dk2_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_dk2_load), MP_ROM_PTR(&rz_drp_dk2_load_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2grayscale_3), MP_ROM_PTR(&simple_isp_bayer2grayscale_3_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2grayscale_6), MP_ROM_PTR(&simple_isp_bayer2grayscale_6_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2rgb_6), MP_ROM_PTR(&simple_isp_bayer2rgb_6_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2yuv_3), MP_ROM_PTR(&simple_isp_bayer2yuv_3_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2yuv_6), MP_ROM_PTR(&simple_isp_bayer2yuv_6_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2yuv_planar_3), MP_ROM_PTR(&simple_isp_bayer2yuv_planar_3_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_bayer2yuv_planar_6), MP_ROM_PTR(&simple_isp_bayer2yuv_planar_6_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_grayscale_3), MP_ROM_PTR(&simple_isp_grayscale_3_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_isp_grayscale_6), MP_ROM_PTR(&simple_isp_grayscale_6_obj) },
    { MP_ROM_QSTR(MP_QSTR_TILE_0), MP_ROM_INT(R_DK2_TILE_0) },
    { MP_ROM_QSTR(MP_QSTR_TILE_1), MP_ROM_INT(R_DK2_TILE_1) },
    { MP_ROM_QSTR(MP_QSTR_TILE_2), MP_ROM_INT(R_DK2_TILE_2) },
    { MP_ROM_QSTR(MP_QSTR_TILE_3), MP_ROM_INT(R_DK2_TILE_3) },
    { MP_ROM_QSTR(MP_QSTR_TILE_4), MP_ROM_INT(R_DK2_TILE_4) },
    { MP_ROM_QSTR(MP_QSTR_TILE_5), MP_ROM_INT(R_DK2_TILE_5) },
};
STATIC MP_DEFINE_CONST_DICT(rz_drp_locals_dict, rz_drp_locals_dict_table);

const mp_obj_type_t rz_drp_type = {
    { &mp_type_type },
    .name = MP_QSTR_DRP,
    .print = rz_drp_obj_print,
    .make_new = rz_drp_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&rz_drp_locals_dict,
};

#endif
