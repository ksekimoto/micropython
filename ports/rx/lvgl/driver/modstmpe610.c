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
#include "stmpe610.h"

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

#define STMPE610_BAUDRATE   1000000
#define STMPE610_AVG 4
#define CMD_X_READ  0b10010000
#define CMD_Y_READ  0b11010000

//////////////////////////////////////////////////////////////////////////////
// Module definition
//////////////////////////////////////////////////////////////////////////////

typedef struct _stmpe610_obj_t
{
    mp_obj_base_t base;

    uint32_t baudrate;
    uint32_t spihost;
    uint32_t mode;
    uint16_t spcmd;
    uint8_t spbr;
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
    int16_t avg_buf_x[STMPE610_AVG];
    int16_t avg_buf_y[STMPE610_AVG];
    uint8_t avg_last;

} stmpe610_obj_t;

STATIC mp_obj_t mp_stmpe610_init(mp_obj_t self_in);
STATIC mp_obj_t mp_stmpe610_deinit(mp_obj_t self_in);
static bool stmpe610_read(lv_indev_data_t * data);

// Unfortunately, lvgl doesn't pass user_data to callbacks, so we use this global.
// This means we can have only one active touch driver instance, pointed by this global.
STATIC stmpe610_obj_t *g_stmpe610 = NULL;

STATIC mp_obj_t mp_activate_stmpe610(mp_obj_t self_in)
{
    stmpe610_obj_t *self = MP_OBJ_TO_PTR(self_in);
    g_stmpe610 = self;
    return mp_const_none;
}

STATIC mp_obj_t stmpe610_make_new(const mp_obj_type_t *type,
                               size_t n_args,
                               size_t n_kw,
                               const mp_obj_t *all_args)
{
    enum{
        ARG_baudrate,
        ARG_spihost,
        ARG_mode,
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
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = STMPE610_BAUDRATE}},
        { MP_QSTR_spihost, MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_mode, MP_ARG_INT, {.u_int = 0}},
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
    stmpe610_obj_t *self = m_new_obj(stmpe610_obj_t);
    self->base.type = type;

    self->baudrate = args[ARG_baudrate].u_int;
    self->spihost = args[ARG_spihost].u_int;
    self->mode = args[ARG_mode].u_int;
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

STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_init_stmpe610_obj, mp_stmpe610_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_deinit_stmpe610_obj, mp_stmpe610_deinit);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_activate_stmpe610_obj, mp_activate_stmpe610);
DEFINE_PTR_OBJ(stmpe610_read);

STATIC const mp_rom_map_elem_t stmpe610_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_stmpe610_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&mp_deinit_stmpe610_obj) },
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&mp_activate_stmpe610_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&PTR_OBJ(stmpe610_read)) },
};

STATIC MP_DEFINE_CONST_DICT(stmpe610_locals_dict, stmpe610_locals_dict_table);

STATIC const mp_obj_type_t stmpe610_type = {
    { &mp_type_type },
    .name = MP_QSTR_stmpe610,
    //.print = stmpe610_print,
    .make_new = stmpe610_make_new,
    .locals_dict = (mp_obj_dict_t*)&stmpe610_locals_dict,
};

STATIC const mp_rom_map_elem_t stmpe610_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_stmpe610) },
        { MP_ROM_QSTR(MP_QSTR_stmpe610), (mp_obj_t)&stmpe610_type},
};
         

STATIC MP_DEFINE_CONST_DICT (
    mp_module_stmpe610_globals,
    stmpe610_globals_table
);

const mp_obj_module_t mp_module_stmpe610 = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_stmpe610_globals
};

//////////////////////////////////////////////////////////////////////////////
// Module implementation
//////////////////////////////////////////////////////////////////////////////

STATIC void write_8bit_reg(stmpe610_obj_t *self, uint8_t reg, uint8_t value) {
    uint8_t src[2];
    uint8_t dst[2];
    src[0] = reg;
    src[1] = value;
    uint32_t state = disable_irq();
    SPI_START_XFER(self->spihost, self->spcmd, self->spbr);
    mp_hal_pin_write(self->cs, 0);
    SPI_TRANSFER(self->spihost, 8, (uint8_t *)&dst, (uint8_t *)&src, 2, 1000);
    mp_hal_pin_write(self->cs, 1);
    enable_irq(state);
}

STATIC uint8_t read_8bit_reg(stmpe610_obj_t *self, uint8_t reg) {
    uint8_t src[2];
    uint8_t dst[2];
    src[0] = 0x80 | reg;
    uint32_t state = disable_irq();
    SPI_START_XFER(self->spihost, self->spcmd, self->spbr);
    mp_hal_pin_write(self->cs, 0);
    SPI_TRANSFER(self->spihost, 8, (uint8_t *)&dst, (uint8_t *)&src, 2, 1000);
    mp_hal_pin_write(self->cs, 1);
    enable_irq(state);
    return dst[1];
}

STATIC uint16_t read_16bit_reg(stmpe610_obj_t *self, uint8_t reg) {
    uint8_t dst[2];
    dst[0] = read_8bit_reg(self, reg);
    dst[1] = read_8bit_reg(self, reg + 1);
    return (uint16_t)dst[0] << 8 | (uint16_t)dst[1];
}

STATIC mp_obj_t mp_stmpe610_init(mp_obj_t self_in)
{
    uint8_t u8 = 0;
    uint16_t u16 = 0;
    stmpe610_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_hal_pin_input(self->irq);
    mp_hal_pin_output(self->cs);
    mp_hal_pin_write(self->cs, 1);
    mp_activate_stmpe610(self_in);
    SPI_INIT((uint32_t)self->spihost, self->cs->pin, self->baudrate, 8, self->mode);
    SPI_GET_CONF((uint32_t)self->spihost, &self->spcmd, &self->spbr);

    write_8bit_reg(self, STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
    mp_hal_delay_ms(100);

    u8 = read_8bit_reg(self, STMPE_SPI_CFG);
    write_8bit_reg(self, STMPE_SPI_CFG, u8 | STMPE_SPI_CFG_AA);
    u8 = read_8bit_reg(self, STMPE_SPI_CFG);

    u16 = read_16bit_reg(self, STMPE_CHIP_ID);
    if (u16 != 0x811) {
#if defined(DEBUG_STMPE610)
        debug_printf(TAG, "Incorrect version: 0x%x", u16);
#endif
        return MP_OBJ_NEW_SMALL_INT(u16);
    }
    write_8bit_reg(self, STMPE_SYS_CTRL2, 0x00); // Disable clocks
    write_8bit_reg(self, STMPE_TSC_CTRL, 0);     // Disable to allow writing
    write_8bit_reg(self, STMPE_TSC_CTRL,
                   STEMP_TSC_CTRL_TRACK_0 |
                   STMPE_TSC_CTRL_XYZ |
                   STMPE_TSC_CTRL_EN);
    write_8bit_reg(self, STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE |
                   STMPE_TSC_CFG_DELAY_1MS |
                   STMPE_TSC_CFG_SETTLE_1MS);
    write_8bit_reg(self, STMPE_TSC_FRACTION_Z, 0x7);
    write_8bit_reg(self, STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
    write_8bit_reg(self, STMPE_SYS_CTRL2, 0x04);                    // GPIO clock off, TSC clock on, ADC clock on
    write_8bit_reg(self, STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_12BIT | STMPE_ADC_CTRL1_80CLK);
    write_8bit_reg(self, STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_3_25MHZ);
    write_8bit_reg(self, STMPE_GPIO_ALT_FUNCT, 0x00);               // Disable GPIO
    write_8bit_reg(self, STMPE_FIFO_TH, 1);                         // Set FIFO threshold
    write_8bit_reg(self, STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);     // Assert FIFO reset
    write_8bit_reg(self, STMPE_FIFO_STA, 0);                        // Deassert FIFO reset
    write_8bit_reg(self, STMPE_INT_EN, 0x00);                       // No interrupts
    write_8bit_reg(self, STMPE_INT_STA, 0xFF);                      // reset all ints
    return MP_OBJ_NEW_SMALL_INT(u16);
}

STATIC mp_obj_t mp_stmpe610_deinit(mp_obj_t self_in) {
    stmpe610_obj_t *self = MP_OBJ_TO_PTR(self_in);
    SPI_DEINIT((uint32_t)self->spihost, self->cs->pin);
    return mp_const_none;
}

static void read_data(stmpe610_obj_t *self, int16_t *x, int16_t *y, uint8_t *z) {
    *x = read_16bit_reg(self, STMPE_TSC_DATA_X);
    *y = read_16bit_reg(self, STMPE_TSC_DATA_Y);
    *z = read_8bit_reg(self, STMPE_TSC_DATA_Z);
}

static bool buffer_empty(stmpe610_obj_t *self) {
    return ((read_8bit_reg(self, STMPE_FIFO_STA) & STMPE_FIFO_STA_EMPTY) == STMPE_FIFO_STA_EMPTY);
}

static void adjust_data(int16_t *x, int16_t *y) {
#if STMPE610_XY_SWAP != 0
    int16_t swap_tmp;
    swap_tmp = *x;
    *x = *y;
    *y = swap_tmp;
#endif

    if ((*x) > STMPE610_X_MIN)
        (*x) -= STMPE610_X_MIN;
    else
        (*x) = 0;

    if ((*y) > STMPE610_Y_MIN)
        (*y) -= STMPE610_Y_MIN;
    else
        (*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES) / (STMPE610_X_MAX - STMPE610_X_MIN);

    (*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES) / (STMPE610_Y_MAX - STMPE610_Y_MIN);

#if STMPE610_X_INV != 0
    (*x) = LV_HOR_RES - (*x);
#endif

#if STMPE610_Y_INV != 0
    (*y) = LV_VER_RES - (*y);
#endif

}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no ore data to be read
 */
static bool stmpe610_read(lv_indev_data_t *data) {
    stmpe610_obj_t *self = MP_OBJ_TO_PTR(g_stmpe610);
    if (!self)
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("stmpe610 instance needs to be created before callback is called!"));
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    bool valid = true;
    int c = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint8_t z;

    if ((read_8bit_reg(self, STMPE_TSC_CTRL) & STMPE_TSC_TOUCHED) == STMPE_TSC_TOUCHED) {
        // Making sure that we read all data and return the latest point
        while (!buffer_empty(self)) {
            read_data(self, &x, &y, &z);
            c++;
        }

        if (c > 0) {
            //ESP_LOGI(TAG, "%d: %d %d %d", c, x, y, z);

            adjust_data(&x, &y);
            last_x = x;
            last_y = y;
            //ESP_LOGI(TAG, "  ==> %d %d", x, y);
        }

        z = read_8bit_reg(self, STMPE_INT_STA);  // Clear interrupts
        z = read_8bit_reg(self, STMPE_FIFO_STA);
        if ((z & STMPE_FIFO_STA_OFLOW) == STMPE_FIFO_STA_OFLOW) {
            // Clear the FIFO if we discover an overflow
            write_8bit_reg(self, STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
            write_8bit_reg(self, STMPE_FIFO_STA, 0); // unreset
        }
    }

    if (c == 0) {
        x = last_x;
        y = last_y;
        valid = false;
    }

    data->point.x = (int16_t)x;
    data->point.y = (int16_t)y;
    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
    return valid;
}

