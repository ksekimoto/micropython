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
#include "lvgl/src/lv_hal/lv_hal_indev.h"
#include "lvgl/src/lv_core/lv_disp.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "mphalport.h"
#include "pin.h"
#if defined(RZA2M)
#include "rza2m_spi.h"
#endif

#if defined(RZA2M)
#define GPIO_SET_OUTPUT     _gpio_mode_output
#define GPIO_SET_INPUT      _gpio_mode_input
#define GPIO_WRITE          _gpio_write
#define SPI_WRITE_BYTE      rz_spi_write_byte
#define SPI_INIT            rz_spi_init
#define SPI_DEINIT          rz_spi_deinit
#define SPI_GET_CONF        rz_spi_get_conf
#define SPI_START_XFER      rz_spi_start_xfer
#define SPI_END_XFER        rz_spi_end_xfer
#define SPI_TRANSFER        rz_spi_transfer
#else
#define GPIO_SET_OUTPUT     gpio_mode_output
#define GPIO_SET_INPUT      gpio_mode_input
#define GPIO_WRITE          gpio_write
#define SPI_WRITE_BYTE      rx_spi_write_byte
#define SPI_INIT            rx_spi_init
#define SPI_DEINIT          rx_spi_deinit
#define SPI_GET_CONF        rx_spi_get_conf
#define SPI_START_XFER      rx_spi_start_xfer
#define SPI_END_XFER        rx_spi_end_xfer
#define SPI_TRANSFER        rx_spi_transfer
#endif

void mp_hal_delay_ms(mp_uint_t ms);

//////////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////////

#define XPT2046_AVG 4
#define CMD_X_READ  0b10010000
#define CMD_Y_READ  0b11010000

//////////////////////////////////////////////////////////////////////////////
// Module definition
//////////////////////////////////////////////////////////////////////////////

typedef struct _xpt2046_obj_t
{
    mp_obj_base_t base;

    uint32_t mhz;
    uint32_t spihost;
    pin_obj_t *cs;
    pin_obj_t *irq;

    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;
    bool x_inv;
    bool y_inv;    
    bool xy_swap;

    spi_device_handle_t spi;
    int16_t avg_buf_x[XPT2046_AVG];
    int16_t avg_buf_y[XPT2046_AVG];
    uint8_t avg_last;

} xpt2046_obj_t;

// Unfortunately, lvgl doesn't pass user_data to callbacks, so we use this global.
// This means we can have only one active touch driver instance, pointed by this global.
STATIC xpt2046_obj_t *g_xpt2046 = NULL;

STATIC mp_obj_t mp_activate_xpt2046(mp_obj_t self_in)
{
    xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    g_xpt2046 = self;
    return mp_const_none;
}

STATIC mp_obj_t xpt2046_make_new(const mp_obj_type_t *type,
                               size_t n_args,
                               size_t n_kw,
                               const mp_obj_t *all_args)
{
    enum{
        ARG_mhz,
        ARG_spihost,
        ARG_cs,
        ARG_irq,

        ARG_x_min,
        ARG_y_min,
        ARG_x_max,
        ARG_y_max,
        ARG_x_inv,
        ARG_y_inv,
        ARG_xy_swap,
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mhz, MP_ARG_INT, {.u_int = 20}},
        { MP_QSTR_spihost, MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_cs, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_irq, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},

        { MP_QSTR_x_min, MP_ARG_INT, {.u_int = 1000}},
        { MP_QSTR_y_min, MP_ARG_INT, {.u_int = 1000}},
        { MP_QSTR_x_max, MP_ARG_INT, {.u_int = 3200}},
        { MP_QSTR_y_max, MP_ARG_INT, {.u_int = 2000}},
        { MP_QSTR_x_inv, MP_ARG_BOOL, {.u_obj = mp_const_true}},
        { MP_QSTR_y_inv, MP_ARG_BOOL, {.u_obj = mp_const_true}},
        { MP_QSTR_xy_swap, MP_ARG_BOOL, {.u_obj = mp_const_false}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    xpt2046_obj_t *self = m_new_obj(xpt2046_obj_t);
    self->base.type = type;

    self->mhz = args[ARG_mhz].u_int;
    self->spihost = args[ARG_spihost].u_int;
    if (args[ARG_cs].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("cs pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_cs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->cs = MP_OBJ_TO_PTR(args[ARG_cs].u_obj);
    }
    if (args[ARG_irq].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("irq pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_irq].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->irq = MP_OBJ_TO_PTR(args[ARG_irq].u_obj);
    }
    self->x_min = args[ARG_x_min].u_int;
    self->y_min = args[ARG_y_min].u_int;
    self->x_max = args[ARG_x_max].u_int;
    self->y_max = args[ARG_y_max].u_int;
    self->x_inv = args[ARG_x_inv].u_bool;
    self->y_inv = args[ARG_y_inv].u_bool;
    self->xy_swap = args[ARG_xy_swap].u_bool;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t mp_xpt2046_init(mp_obj_t self_in);
STATIC mp_obj_t mp_xpt2046_deinit(mp_obj_t self_in);
STATIC bool xpt2046_read(lv_indev_data_t *data);

STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_init_xpt2046_obj, mp_xpt2046_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_deinit_xpt2046_obj, mp_xpt2046_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_activate_xpt2046_obj, mp_activate_xpt2046);
DEFINE_PTR_OBJ(xpt2046_read);

STATIC const mp_rom_map_elem_t xpt2046_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_xpt2046_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_xpt2046_obj) },
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&mp_activate_xpt2046_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&PTR_OBJ(xpt2046_read)) },
};

STATIC MP_DEFINE_CONST_DICT(xpt2046_locals_dict, xpt2046_locals_dict_table);

STATIC const mp_obj_type_t xpt2046_type = {
    { &mp_type_type },
    .name = MP_QSTR_xpt2046,
    //.print = xpt2046_print,
    .make_new = xpt2046_make_new,
    .locals_dict = (mp_obj_dict_t*)&xpt2046_locals_dict,
};

STATIC const mp_rom_map_elem_t xpt2046_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_xpt2046) },
        { MP_ROM_QSTR(MP_QSTR_xpt2046), (mp_obj_t)&xpt2046_type},
};
         

STATIC MP_DEFINE_CONST_DICT (
    mp_module_xpt2046_globals,
    xpt2046_globals_table
);

const mp_obj_module_t mp_module_xpt2046 = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_xpt2046_globals
};

//////////////////////////////////////////////////////////////////////////////
// Module implementation
//////////////////////////////////////////////////////////////////////////////

STATIC mp_obj_t mp_xpt2046_init(mp_obj_t self_in)
{
    xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_hal_pin_input(self->irq);
    mp_hal_pin_output(self->cs);
    mp_hal_pin_write(self->cs, 1);
    mp_activate_xpt2046(self_in);
    SPI_INIT((uint32_t)self->spihost, self->cs->pin, 20000000, 8, 1);
    return mp_const_none;
}

STATIC mp_obj_t mp_xpt2046_deinit(mp_obj_t self_in)
{
    xpt2046_obj_t *self = MP_OBJ_TO_PTR(self_in);
    SPI_DEINIT((uint32_t)self->spihost, self->cs->pin);
    return mp_const_none;
}

static void xpt2046_corr(xpt2046_obj_t *self, int16_t * x, int16_t * y);
static void xpt2046_avg(xpt2046_obj_t *self, int16_t * x, int16_t * y);
static uint8_t tp_spi_xchg(xpt2046_obj_t *self, uint8_t data_send);

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no ore data to be read
 */
static bool xpt2046_read(lv_indev_data_t * data)
{
    xpt2046_obj_t *self = MP_OBJ_TO_PTR(g_xpt2046 );
    if (!self) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("xpt2046 instance needs to be created before callback is called!"));
    }
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    bool valid = true;
    uint8_t buf;

    int16_t x = 0;
    int16_t y = 0;

    uint8_t irq = (uint8_t)mp_hal_pin_read(self->irq);

    if(irq == 0) {
        mp_hal_pin_write(self->cs, 0);
        tp_spi_xchg(self, CMD_X_READ);         /*Start x read*/

        buf = tp_spi_xchg(self, 0);           /*Read x MSB*/
        x = buf << 8;
        buf = tp_spi_xchg(self, CMD_Y_READ);  /*Until x LSB converted y command can be sent*/
        x += buf;

        buf =  tp_spi_xchg(self, 0);   /*Read y MSB*/
        y = buf << 8;

        buf =  tp_spi_xchg(self, 0);   /*Read y LSB*/
        y += buf;
        mp_hal_pin_write(self->cs, 1);

        /*Normalize Data*/
        x = x >> 3;
        y = y >> 3;
        xpt2046_corr(self, &x, &y);
        xpt2046_avg(self, &x, &y);
        last_x = x;
        last_y = y;


    } else {
        x = last_x;
        y = last_y;
        self->avg_last = 0;
        valid = false;
    }

    data->point.x = x;
    data->point.y = y;
    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;

    return valid;
}

/**********************
 *   HELPER FUNCTIONS
 **********************/

static uint8_t tp_spi_xchg(xpt2046_obj_t *self, uint8_t data_send)
{
    uint8_t data_rec = 0;
    //mp_hal_pin_write(self->cs, 0);
    SPI_TRANSFER(self->spihost, 8, (uint8_t *)&data_rec, (uint8_t *)&data_send, 1, 1000);
    //mp_hal_pin_write(self->cs, 1);
	return data_rec;
}

static void xpt2046_corr(xpt2046_obj_t *self, int16_t * x, int16_t * y)
{
    if (self->xy_swap){
        int16_t swap_tmp;
        swap_tmp = *x;
        *x = *y;
        *y = swap_tmp;
    }

    if((*x) > self->x_min)(*x) -= self->x_min;
    else(*x) = 0;

    if((*y) > self->y_min)(*y) -= self->y_min;
    else(*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES) /
           (self->x_max - self->x_min);

    (*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES) /
           (self->y_max - self->y_min);

    if (self->x_inv){
        (*x) =  LV_HOR_RES - (*x);
    }

    if (self->y_inv){
        (*y) =  LV_VER_RES - (*y);
    }
}


static void xpt2046_avg(xpt2046_obj_t *self, int16_t * x, int16_t * y)
{
    /*Shift out the oldest data*/
    uint8_t i;
    for(i = XPT2046_AVG - 1; i > 0 ; i--) {
        self->avg_buf_x[i] = self->avg_buf_x[i - 1];
        self->avg_buf_y[i] = self->avg_buf_y[i - 1];
    }

    /*Insert the new point*/
    self->avg_buf_x[0] = *x;
    self->avg_buf_y[0] = *y;
    if(self->avg_last < XPT2046_AVG) self->avg_last++;

    /*Sum the x and y coordinates*/
    int32_t x_sum = 0;
    int32_t y_sum = 0;
    for(i = 0; i < self->avg_last ; i++) {
        x_sum += self->avg_buf_x[i];
        y_sum += self->avg_buf_y[i];
    }

    /*Normalize the sums*/
    (*x) = (int32_t)x_sum / self->avg_last;
    (*y) = (int32_t)y_sum / self->avg_last;
}
