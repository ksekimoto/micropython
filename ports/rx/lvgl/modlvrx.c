#include "py/obj.h"
#include "py/mpstate.h"
#include "lv_mpy.h"
#include "pendsv.h"
#if defined(RX65N)
#include "pendsv.h"
#endif

#if LVGL_ENABLE

/* Defines the LittlevGL tick rate in milliseconds. */
/* Increasing this value might help with CPU usage at the cost of lower
 * responsiveness. */
#define LV_TICK_RATE 5

//////////////////////////////////////////////////////////////////////////////

#define MONITOR_HOR_RES     LCD_PIXEL_WIDTH
#define MONITOR_VER_RES     LCD_PIXEL_HEIGHT

int lvrx_enable = 0;

/**
 * Flush a buffer to the display. Calls 'lv_flush_ready()' when finished
 */
void lvrx_flush(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    /*Return if the area is out the screen*/
    int32_t x;
    int32_t y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
#if 0
            mbed_set_pixel(x, y, (*color_p).full);
#endif
            color_p++;
        }
    }
    // Clean dcache to flush display buffer
    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);

#if 0
    if(area->x2 < 0 || area->y2 < 0 || area->x1 > MONITOR_HOR_RES - 1 || area->y1 > MONITOR_VER_RES - 1) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    int32_t y;
#if LV_COLOR_DEPTH != 24 && LV_COLOR_DEPTH != 32    /*32 is valid but support 24 for backward compatibility too*/
    int32_t x;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            if (tft_fb) {
                tft_fb[y * MONITOR_HOR_RES + x] = lv_color_to32(*color_p);
            }
            color_p++;
        }

    }
#else
    uint32_t w = area->x2 - area->x1 + 1;
    for(y = area->y1; y <= area->y2; y++) {
        if (tft_fb_ptr) {
            memcpy(&tft_fb_ptr[y * MONITOR_HOR_RES + area->x1], color_p, w * sizeof(lv_color_t));
        }
        color_p += w;
    }
#endif
    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);
#endif
}

STATIC mp_obj_t mp_lv_task_handler(mp_obj_t arg) {
    lv_task_handler();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_lv_task_handler_obj, mp_lv_task_handler);

void tick_thread(void) {
    if (lvrx_enable) {
        if ((mtick() % LV_TICK_RATE) == 0) {
            lv_tick_inc(LV_TICK_RATE);
            mp_sched_schedule((mp_obj_t)&mp_lv_task_handler_obj, mp_const_none);
        }
        pendsv_schedule_dispatch(PENDSV_DISPATCH_LV, tick_thread);
    }
}

STATIC mp_obj_t mp_init_lvrx(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_w, ARG_h };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_w, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LV_HOR_RES_MAX} },
        { MP_QSTR_h, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LV_VER_RES_MAX} },
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    lvrx_enable = 1;
#if defined(RX65N)
    pendsv_schedule_dispatch(PENDSV_DISPATCH_LV, tick_thread);
#endif
    return mp_const_none;
}

STATIC mp_obj_t mp_deinit_lvrx() {
    lvrx_enable = 0;
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_init_lvrx_obj, 0, mp_init_lvrx);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_deinit_lvrx_obj, mp_deinit_lvrx);

DEFINE_PTR_OBJ(lvrx_flush);
//DEFINE_PTR_OBJ(lvrx_read);

STATIC const mp_rom_map_elem_t LVRX_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvrx) },
        { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_lvrx_obj) },
        { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_lvrx_obj) },
        { MP_ROM_QSTR(MP_QSTR_monitor_flush), MP_ROM_PTR(&PTR_OBJ(lvrx_flush))},
//        { MP_ROM_QSTR(MP_QSTR_mouse_read), MP_ROM_PTR(&PTR_OBJ(lvr_read))},
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_lvrx_globals,
    LVRX_globals_table
);

const mp_obj_module_t mp_module_lvrx = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_lvrx_globals
};

#endif
