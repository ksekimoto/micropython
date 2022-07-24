/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
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

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objint.h"
#include "extmod/machine_mem.h"
#include "portmodules.h"
#include "rz_lcd.h"
#include "rz_camera.h"

#if MICROPY_PY_RZ

#if MICROPY_PY_PYB_FONT
extern const mp_obj_type_t rz_font_type;
#endif
#if MICROPY_PY_PYB_LCDSPI
extern const mp_obj_type_t rz_lcdspi_type;
#endif
extern const mp_obj_type_t rz_display_type;
extern const mp_obj_type_t rz_lcd_type;
extern const mp_obj_type_t rz_camera_type;
extern const mp_obj_type_t rz_drp_type;
extern const mp_obj_type_t rz_isp_type;

STATIC const mp_rom_map_elem_t rz_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rz) },

    { MP_ROM_QSTR(MP_QSTR_mem8), MP_ROM_PTR(&machine_mem8_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem16), MP_ROM_PTR(&machine_mem16_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem32), MP_ROM_PTR(&machine_mem32_obj) },

    #if MICROPY_PY_PYB_FONT
    { MP_ROM_QSTR(MP_QSTR_FONT), MP_ROM_PTR(&rz_font_type) },
    #endif
    #if MICROPY_PY_PYB_LCDSPI
    { MP_ROM_QSTR(MP_QSTR_LCDSPI), MP_ROM_PTR(&rz_lcdspi_type) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_DISPLAY), MP_ROM_PTR(&rz_display_type) },
    { MP_ROM_QSTR(MP_QSTR_LCD), MP_ROM_PTR(&rz_lcd_type) },
    { MP_ROM_QSTR(MP_QSTR_CAMERA), MP_ROM_PTR(&rz_camera_type) },
    #if USE_DRP
    { MP_ROM_QSTR(MP_QSTR_DRP), MP_ROM_PTR(&rz_drp_type) },
    { MP_ROM_QSTR(MP_QSTR_ISP), MP_ROM_PTR(&rz_isp_type) },
    #endif
};
STATIC MP_DEFINE_CONST_DICT(rz_module_globals, rz_module_globals_table);

const mp_obj_module_t rz_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&rz_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_rz, rz_module);

#endif // MICROPY_PY_RZ
