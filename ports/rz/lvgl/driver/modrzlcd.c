/*
 * Portion Copyright (c) 2020, Kentaro Sekimoto
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

#include "lv_mpy.h"
#include "lvgl/src/lv_hal/lv_hal_disp.h"
#include "py/obj.h"
#include "py/runtime.h"
// #include "dcache-control.h"
#include "mbed_camera.h"

extern void dcache_clean(void *p_buf, uint32_t size);
extern void dcache_invalid(void *p_buf, uint32_t size);
void lvrz_set_pixel(int32_t x, int32_t y, uint16_t color);
uint8_t *lvrz_get_lcd_fb_ptr(void);
uint32_t lvrz_get_lcd_fb_size(void);

//////////////////////////////////////////////////////////////////////////////
// rzlcd requires specific lv_conf resolution and color depth
//////////////////////////////////////////////////////////////////////////////

#if LV_COLOR_DEPTH != 16
#error "modrzlcd: LV_COLOR_DEPTH must be set to 16!"
#endif

//////////////////////////////////////////////////////////////////////////////
// rzlcd Module definitions
//////////////////////////////////////////////////////////////////////////////

#define MONITOR_HOR_RES     LCD_PIXEL_WIDTH
#define MONITOR_VER_RES     LCD_PIXEL_HEIGHT

typedef struct {
    mp_obj_base_t base;
    uint32_t lcd_type;
} rzlcd_t;


static uint8_t *tft_fb_ptr = 0;
static uint32_t tft_fb_size = 0;

// Unfortunately, lvgl doesnt pass user_data to callbacks, so we use this global.
// This means we can have only one active display driver instance, pointed by this global.
STATIC rzlcd_t *g_rzlcd = NULL;

STATIC mp_obj_t rzlcd_make_new(const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *all_args);

STATIC mp_obj_t mp_init_rzlcd(mp_obj_t self_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_init_rzlcd_obj, mp_init_rzlcd);

STATIC mp_obj_t mp_activate_rzlcd(mp_obj_t self_in) {
    rzlcd_t *self = MP_OBJ_TO_PTR(self_in);
    g_rzlcd = self;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_activate_rzlcd_obj, mp_activate_rzlcd);

STATIC void rzlcd_flush(struct _disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
DEFINE_PTR_OBJ(rzlcd_flush);

STATIC const mp_rom_map_elem_t rzlcd_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_rzlcd_obj) },
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&mp_activate_rzlcd_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&PTR_OBJ(rzlcd_flush)) },
};
STATIC MP_DEFINE_CONST_DICT(rzlcd_locals_dict, rzlcd_locals_dict_table);

STATIC const mp_obj_type_t rzlcd_type = {
    { &mp_type_type },
    .name = MP_QSTR_rzlcd,
    // .print = rzlcd_print,
    .make_new = rzlcd_make_new,
    .locals_dict = (mp_obj_dict_t *)&rzlcd_locals_dict,
};

STATIC mp_obj_t rzlcd_make_new(const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *all_args) {
    enum {
        ARG_lcd_type,
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_lcd_type, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    rzlcd_t *self = m_new_obj(rzlcd_t);
    self->base.type = type;
    self->lcd_type = args[ARG_lcd_type].u_int;
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t rzlcd_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rzlcd) },
    { MP_ROM_QSTR(MP_QSTR_display), (mp_obj_t)&rzlcd_type},
};


STATIC MP_DEFINE_CONST_DICT(
    mp_module_rzlcd_globals,
    rzlcd_globals_table
    );

const mp_obj_module_t mp_module_rzlcd = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_rzlcd_globals
};

//////////////////////////////////////////////////////////////////////////////
// rzlcd driver implementation
//////////////////////////////////////////////////////////////////////////////

STATIC void rzlcd_clear(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t col) {
    // rzlcd_t *self = g_rzlcd;

    /* ToDo */
}

STATIC mp_obj_t mp_init_rzlcd(mp_obj_t self_in) {
    // rzlcd_t *self = MP_OBJ_TO_PTR(self_in);
    mp_activate_rzlcd(self_in);

    tft_fb_ptr = lvrz_get_lcd_fb_ptr();
    tft_fb_size = lvrz_get_lcd_fb_size();
    // rzlcd_clear(0, 0, 239, 319, 0);
    mbed_lcd_init();

    return mp_const_none;
}

/**
 * Flush a buffer to the display. Calls 'lv_flush_ready()' when finished
 */
STATIC void rzlcd_flush(struct _disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    // rzlcd_t *self = g_rzlcd;
    /* ToDo */
    /*Return if the area is out the screen*/
    int32_t x;
    int32_t y;
    for (y = area->y1; y <= area->y2; y++) {
        for (x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            lvrz_set_pixel(x, y, (*color_p).full);
            color_p++;
        }
    }
    // Clean dcache to flush display buffer
    dcache_clean((void *)tft_fb_ptr, tft_fb_size);
    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);
}

void lvrz_flush(struct _disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
}
