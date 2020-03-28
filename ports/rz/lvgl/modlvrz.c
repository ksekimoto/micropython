#include "py/obj.h"
#include "py/mpstate.h"
#include "lv_mpy.h"
#include "dcache-control.h"
#include "mbed_camera_lcd.h"
#include "pendsv.h"

//#if LVGL_ENABLE

/* Defines the LittlevGL tick rate in milliseconds. */
/* Increasing this value might help with CPU usage at the cost of lower
 * responsiveness. */
#define LV_TICK_RATE 50

//////////////////////////////////////////////////////////////////////////////

#define MONITOR_HOR_RES     LCD_PIXEL_WIDTH
#define MONITOR_VER_RES     LCD_PIXEL_HEIGHT

static uint8_t *tft_fb_ptr = 0;
static uint32_t tft_fb_size = 0;

bool lvrz_active(void) {
    return true;
}

/**
 * Flush a buffer to the display. Calls 'lv_flush_ready()' when finished
 */
void lvrz_flush(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    /*Return if the area is out the screen*/
    int32_t x;
    int32_t y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            mbed_set_pixel(x, y, (*color_p).full);
            color_p++;
        }
    }
    // Clean dcache to flush display buffer
    dcache_clean((void *)tft_fb_ptr, tft_fb_size);
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
    dcache_clean((void *)tft_fb_ptr, tft_fb_size);
    /*IMPORTANT! It must be called to tell the system the flush is ready*/
    lv_disp_flush_ready(disp_drv);
#endif
}

STATIC mp_obj_t mp_lv_task_handler(mp_obj_t arg) {
    lv_task_handler();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_lv_task_handler_obj, mp_lv_task_handler);

STATIC int tick_thread(void * data) {
    (void)data;
    lv_tick_inc(1);
    lv_task_handler();
#if 0
    if (lvrz_active()) {
        lv_tick_inc(1); /*Tell LittelvGL that 1 milliseconds were elapsed*/
        mp_sched_schedule((mp_obj_t)&mp_lv_task_handler_obj, mp_const_none);
        if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE) {
            MP_STATE_VM(sched_state) = MP_SCHED_PENDING;
        }
        pendsv_schedule_dispatch(PENDSV_DISPATCH_LVRZ, tick_thread);
    }
#endif
    return 0;
}

STATIC mp_obj_t mp_init_lvrz(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_w, ARG_h };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_w, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LV_HOR_RES_MAX} },
        { MP_QSTR_h, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = LV_VER_RES_MAX} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    //monitor_init(args[ARG_w].u_int, args[ARG_h].u_int);
    tft_fb_ptr= mbed_get_fb_ptr();
    tft_fb_size = mbed_get_fb_size();
    mbed_lcd_init();
    mbed_ticker_thread((void *)tick_thread, 1000);
    return mp_const_none;
}

STATIC mp_obj_t mp_deinit_lvrz() {
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_init_lvrz_obj, 0, mp_init_lvrz);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_deinit_lvrz_obj, mp_deinit_lvrz);

DEFINE_PTR_OBJ(lvrz_flush);
//DEFINE_PTR_OBJ(lvrz_read);

STATIC const mp_rom_map_elem_t LVRZ_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvrz) },
        { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_lvrz_obj) },
        { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_lvrz_obj) },
        { MP_ROM_QSTR(MP_QSTR_monitor_flush), MP_ROM_PTR(&PTR_OBJ(lvrz_flush))},
//        { MP_ROM_QSTR(MP_QSTR_mouse_read), MP_ROM_PTR(&PTR_OBJ(lvr_read))},
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_lvrz_globals,
    LVRZ_globals_table
);

const mp_obj_module_t mp_module_lvrz = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_lvrz_globals
};

//#endif
