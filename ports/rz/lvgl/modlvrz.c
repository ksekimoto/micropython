#include "py/obj.h"
#include "py/mpstate.h"
#include "lv_mpy.h"
#include "irq.h"
#include "pendsv.h"
#include "mbed_camera.h"

#if LVGL_ENABLE

//#define USE_PENDSV

/* Defines the LittlevGL tick rate in milliseconds. */
/* Increasing this value might help with CPU usage at the cost of lower
 * responsiveness. */
#define LV_TICK_RATE 20

//////////////////////////////////////////////////////////////////////////////

int lvrx_enable = 0;

bool lvrz_active(void) {
    return true;
}

STATIC mp_obj_t mp_lv_task_handler(mp_obj_t arg) {
    lv_task_handler();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_lv_task_handler_obj, mp_lv_task_handler);

void tick_thread(void) {
#if defined(USE_PENDSV)
    if (lvrx_enable) {
        if ((mtick() % LV_TICK_RATE) == 0) {
            lv_tick_inc(LV_TICK_RATE);
            mp_sched_schedule((mp_obj_t)&mp_lv_task_handler_obj, mp_const_none);
        }
        pendsv_schedule_dispatch(PENDSV_DISPATCH_LV, tick_thread);
    }
#else
    if (lvrx_enable) {
        lv_tick_inc(LV_TICK_RATE);
        mp_sched_schedule((mp_obj_t)&mp_lv_task_handler_obj, mp_const_none);
    }
#endif
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
    lvrx_enable = 1;
    //monitor_init(args[ARG_w].u_int, args[ARG_h].u_int);
#if defined(USE_PENDSV)
    pendsv_schedule_dispatch(PENDSV_DISPATCH_LV, tick_thread);
#else
    mbed_ticker_thread((void *)tick_thread, 1000*LV_TICK_RATE);
#endif
    return mp_const_none;
}

STATIC mp_obj_t mp_deinit_lvrz() {
    lvrx_enable = 0;
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_init_lvrz_obj, 0, mp_init_lvrz);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_deinit_lvrz_obj, mp_deinit_lvrz);

//DEFINE_PTR_OBJ(lvrz_read);

STATIC const mp_rom_map_elem_t LVRZ_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvrz) },
        { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_lvrz_obj) },
        { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_lvrz_obj) },
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

#endif
