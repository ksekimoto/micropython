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
#include "r_dk2_if.h"
#include "r_dk2_core.h"
#include "r_drp_simple_isp.h"
#include "rz_buf.h"
#include "rz_isp.h"

#if USE_DRP
// ==================================================================
// ISP class
// ==================================================================

STATIC mp_obj_t rz_isp_src(size_t n_args, const mp_obj_t *args) {
    rz_isp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t src;
    if (n_args == 2) {
        self->param_isp->src = (uint32_t)mp_obj_get_int(args[1]);
    }
    src = self->param_isp->src;
    return mp_obj_new_int_from_uint((mp_uint_t)src);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_isp_src_obj, 1, 2, rz_isp_src);

STATIC mp_obj_t rz_isp_dst(size_t n_args, const mp_obj_t *args) {
    rz_isp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t dst;
    if (n_args == 2) {
        self->param_isp->dst = (uint32_t)mp_obj_get_int(args[1]);
    }
    dst = self->param_isp->dst;
    return mp_obj_new_int_from_uint((mp_uint_t)dst);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_isp_dst_obj, 1, 2, rz_isp_dst);

STATIC mp_obj_t rz_isp_width(size_t n_args, const mp_obj_t *args) {
    rz_isp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t width;
    if (n_args == 2) {
        self->param_isp->width = (uint32_t)mp_obj_get_int(args[1]);
    }
    width = self->param_isp->width;
    return mp_obj_new_int_from_uint((mp_uint_t)width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_isp_width_obj, 1, 2, rz_isp_width);

STATIC mp_obj_t rz_isp_height(size_t n_args, const mp_obj_t *args) {
    rz_isp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint32_t height;
    if (n_args == 2) {
        self->param_isp->height = (uint32_t)mp_obj_get_int(args[1]);
    }
    height = self->param_isp->height;
    return mp_obj_new_int_from_uint((mp_uint_t)height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_isp_height_obj, 1, 2, rz_isp_height);

STATIC mp_obj_t rz_isp_gain(size_t n_args, const mp_obj_t *args) {
    rz_isp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args != 1) {
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[0], 3, &items);
        self->param_isp->gain_r = (uint16_t)mp_obj_get_int(items[0]);
        self->param_isp->gain_g = (uint16_t)mp_obj_get_int(items[1]);
        self->param_isp->gain_b = (uint16_t)mp_obj_get_int(items[2]);
    }
    mp_obj_t tuple[3] = {
        MP_OBJ_NEW_SMALL_INT((int)self->param_isp->gain_r),
        MP_OBJ_NEW_SMALL_INT((int)self->param_isp->gain_g),
        MP_OBJ_NEW_SMALL_INT((int)self->param_isp->gain_b),
    };
    return mp_obj_new_tuple(3, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_isp_gain_obj, 1, 2, rz_isp_gain);

STATIC mp_obj_t rz_isp_bias(size_t n_args, const mp_obj_t *args) {
    rz_isp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if (n_args != 1) {
        mp_obj_t *items;
        mp_obj_get_array_fixed_n(args[0], 3, &items);
        self->param_isp->bias_r = (uint16_t)mp_obj_get_int(items[0]);
        self->param_isp->bias_g = (uint16_t)mp_obj_get_int(items[1]);
        self->param_isp->bias_b = (uint16_t)mp_obj_get_int(items[2]);
    }
    mp_obj_t tuple[3] = {
        MP_OBJ_NEW_SMALL_INT((int)self->param_isp->bias_r),
        MP_OBJ_NEW_SMALL_INT((int)self->param_isp->bias_g),
        MP_OBJ_NEW_SMALL_INT((int)self->param_isp->bias_b),
    };
    return mp_obj_new_tuple(3, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(rz_isp_bias_obj, 1, 2, rz_isp_bias);

STATIC void rz_isp_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    // rz_isp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ISP");
}

/// \classmethod \constructor(id)
/// Create an isp object
///

STATIC mp_obj_t rz_isp_init_helper(rz_isp_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_accumulate, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_table, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area1_offset_x, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area1_offset_y, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area1_width, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area1_height, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area2_offset_x, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area2_offset_y, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area2_width, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area2_height, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area3_offset_x, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area3_offset_y, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area3_width, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_area3_height, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_gain_r, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_gain_g, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_gain_b, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_blend, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bias_r, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bias_g, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bias_b, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_gamma, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_component, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_strength, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_coring, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bias_gray, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_gain_gray, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst2, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst3, MP_ARG_INT, {.u_int = 0} },
    };
    // parse args
    struct {
        mp_arg_val_t src, dst, accumulate, table, width, height,
                     area1_offset_x, area1_offset_y, area1_width, area1_height,
                     area2_offset_x, area2_offset_y, area2_width, area2_height,
                     area3_offset_x, area3_offset_y, area3_width, area3_height,
                     gain_r, gain_g, gain_b, blend, bias_r, bias_g, bias_b, gamma,
                     component, strength, coring, bias_gray, gain_gray, dst2, dst3;
    } args;
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t *)&args);

    r_drp_simple_isp_t *p = self->param_isp;
    p->src = (uint32_t)args.src.u_int;
    p->dst = (uint32_t)args.dst.u_int;
    p->accumulate = (uint32_t)args.accumulate.u_int;
    p->table = (uint32_t)args.table.u_int;
    p->width = (uint16_t)args.width.u_int;
    p->height = (uint16_t)args.height.u_int;
    p->area1_offset_x = (uint16_t)args.area1_offset_x.u_int;
    p->area1_offset_y = (uint16_t)args.area1_offset_y.u_int;
    p->area1_width = (uint16_t)args.area1_width.u_int;
    p->area1_height = (uint16_t)args.area1_height.u_int;
    p->area2_offset_x = (uint16_t)args.area2_offset_x.u_int;
    p->area2_offset_y = (uint16_t)args.area2_offset_y.u_int;
    p->area2_width = (uint16_t)args.area2_width.u_int;
    p->area2_height = (uint16_t)args.area2_height.u_int;
    p->area3_offset_x = (uint16_t)args.area3_offset_x.u_int;
    p->area3_offset_y = (uint16_t)args.area3_offset_y.u_int;
    p->area3_width = (uint16_t)args.area3_width.u_int;
    p->area3_height = (uint16_t)args.area3_height.u_int;
    p->gain_r = (uint16_t)args.gain_r.u_int;
    p->gain_g = (uint16_t)args.gain_b.u_int;
    p->gain_b = (uint16_t)args.gain_b.u_int;
    p->blend = (uint16_t)args.blend.u_int;
    p->bias_r = (uint16_t)args.bias_r.u_int;
    p->bias_g = (uint16_t)args.bias_g.u_int;
    p->bias_b = (uint16_t)args.bias_b.u_int;
    p->gamma = (uint16_t)args.gamma.u_int;
    p->component = (uint16_t)args.component.u_int;
    p->strength = (uint16_t)args.strength.u_int;
    p->coring = (uint16_t)args.coring.u_int;
    p->bias_gray = (uint16_t)args.bias_gray.u_int;
    p->gain_gray = (uint16_t)args.gain_gray.u_int;
    p->dst2 = (uint16_t)args.dst2.u_int;
    p->dst3 = (uint16_t)args.dst3.u_int;
    return mp_const_none;
}

STATIC mp_obj_t rz_isp_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    rz_isp_obj_t *isp;
    isp = m_new_obj(rz_isp_obj_t);
    memset(isp, 0, sizeof(*isp));
    isp->base.type = &rz_isp_type;
    isp->param_isp = (r_drp_simple_isp_t *)rz_malloc(sizeof(r_drp_simple_isp_t));
    if (isp->param_isp == (r_drp_simple_isp_t *)NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("ISP alloc failed"));
    }
    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        rz_isp_init_helper(isp, n_args - 1, args + 1, &kw_args);
    }
    return MP_OBJ_FROM_PTR(isp);
}

STATIC const mp_rom_map_elem_t rz_isp_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_src), MP_ROM_PTR(&rz_isp_src_obj) },
    { MP_ROM_QSTR(MP_QSTR_dst), MP_ROM_PTR(&rz_isp_dst_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&rz_isp_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&rz_isp_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_gain), MP_ROM_PTR(&rz_isp_gain_obj) },
    { MP_ROM_QSTR(MP_QSTR_bias), MP_ROM_PTR(&rz_isp_bias_obj) },
};
STATIC MP_DEFINE_CONST_DICT(rz_isp_locals_dict, rz_isp_locals_dict_table);

const mp_obj_type_t rz_isp_type = {
    { &mp_type_type },
    .name = MP_QSTR_ISP,
    .print = rz_isp_obj_print,
    .make_new = rz_isp_obj_make_new,
    .locals_dict = (mp_obj_dict_t *)&rz_isp_locals_dict,
};

#endif
