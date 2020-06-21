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
#define SPI_GET_CONF        rx_spi_get_conf
#define SPI_START_XFER      rx_spi_start_xfer
#define SPI_END_XFER        rx_spi_end_xfer
#define SPI_TRANSFER        rx_spi_transfer
#endif

void mp_hal_delay_ms(mp_uint_t ms);

//////////////////////////////////////////////////////////////////////////////
// ILI9341 requires specific lv_conf resolution and color depth
//////////////////////////////////////////////////////////////////////////////

#if LV_COLOR_DEPTH != 16
#error "modILI9341: LV_COLOR_DEPTH must be set to 16!"
#endif

//////////////////////////////////////////////////////////////////////////////
// ILI9341 Module definitions
//////////////////////////////////////////////////////////////////////////////

typedef struct {
    mp_obj_base_t base;
    spi_device_handle_t *spi;
    uint32_t baudrate;
    uint32_t spihost;
    uint32_t mode;
    uint16_t spcmd;
    uint8_t spbr;
    pin_obj_t *miso;
    pin_obj_t *mosi;
    pin_obj_t *clk;
    pin_obj_t *cs;
    pin_obj_t *dc;
    pin_obj_t *rst;
    pin_obj_t *backlight;

} ILI9341_t;

// Unfortunately, lvgl doesnt pass user_data to callbacks, so we use this global.
// This means we can have only one active display driver instance, pointed by this global.
STATIC ILI9341_t *g_ILI9341 = NULL;

STATIC mp_obj_t ILI9341_make_new(const mp_obj_type_t *type,
                                 size_t n_args,
                                 size_t n_kw,
                                 const mp_obj_t *all_args);

STATIC mp_obj_t mp_init_ILI9341(mp_obj_t self_in);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_init_ILI9341_obj, mp_init_ILI9341);

STATIC mp_obj_t mp_activate_ILI9341(mp_obj_t self_in)
{
    ILI9341_t *self = MP_OBJ_TO_PTR(self_in);
    g_ILI9341 = self;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_activate_ILI9341_obj, mp_activate_ILI9341);

STATIC void ili9431_flush(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
DEFINE_PTR_OBJ(ili9431_flush);

STATIC const mp_rom_map_elem_t ILI9341_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mp_init_ILI9341_obj) },
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&mp_activate_ILI9341_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&PTR_OBJ(ili9431_flush)) },
};
STATIC MP_DEFINE_CONST_DICT(ILI9341_locals_dict, ILI9341_locals_dict_table);

STATIC const mp_obj_type_t ILI9341_type = {
    { &mp_type_type },
    .name = MP_QSTR_ILI9341,
    //.print = ILI9341_print,
    .make_new = ILI9341_make_new,
    .locals_dict = (mp_obj_dict_t*)&ILI9341_locals_dict,
};

STATIC mp_obj_t ILI9341_make_new(const mp_obj_type_t *type,
                                 size_t n_args,
                                 size_t n_kw,
                                 const mp_obj_t *all_args)
{
    enum{
         ARG_baudrate,
         ARG_spihost,
         ARG_mode,
         ARG_miso,
         ARG_mosi,
         ARG_clk,
         ARG_cs,
         ARG_dc,
         ARG_rst,
         ARG_backlight,
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate,MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int=24000000}},
        { MP_QSTR_spihost,MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int=0}},
        { MP_QSTR_mode,MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int=1}},
        { MP_QSTR_miso, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_mosi, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_clk, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_cs, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_dc, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_rst, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        { MP_QSTR_backlight, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    ILI9341_t *self = m_new_obj(ILI9341_t);
    self->base.type = type;
    self->spi = NULL;
    self->baudrate = args[ARG_baudrate].u_int;
    self->spihost = args[ARG_spihost].u_int;
    self->mode = args[ARG_mode].u_int;

    if (args[ARG_miso].u_obj == MP_OBJ_NULL) {
        //nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "miso pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_miso].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->miso = MP_OBJ_TO_PTR(args[ARG_miso].u_obj);
    }
    if (args[ARG_mosi].u_obj == MP_OBJ_NULL) {
        //nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "mosi pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_mosi].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->mosi = MP_OBJ_TO_PTR(args[ARG_mosi].u_obj);
    }
    if (args[ARG_clk].u_obj == MP_OBJ_NULL) {
        //nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "clk pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_clk].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->clk = MP_OBJ_TO_PTR(args[ARG_clk].u_obj);
    }
    if (args[ARG_cs].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("cs pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_cs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->cs = MP_OBJ_TO_PTR(args[ARG_cs].u_obj);
    }
    if (args[ARG_dc].u_obj == MP_OBJ_NULL) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("dc pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_dc].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->dc = MP_OBJ_TO_PTR(args[ARG_dc].u_obj);
    }
    if (args[ARG_rst].u_obj == MP_OBJ_NULL) {
        //nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "rst pin not specified"));
        self->rst = (pin_obj_t *)0;
    } else if (!mp_obj_is_type(args[ARG_rst].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->rst = MP_OBJ_TO_PTR(args[ARG_rst].u_obj);
    }
    if (args[ARG_backlight].u_obj == MP_OBJ_NULL) {
        //nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "backlight pin not specified"));
    } else if (!mp_obj_is_type(args[ARG_backlight].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->backlight = MP_OBJ_TO_PTR(args[ARG_backlight].u_obj);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t ILI9341_globals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ILI9341) },
        { MP_ROM_QSTR(MP_QSTR_display), (mp_obj_t)&ILI9341_type},
};
         

STATIC MP_DEFINE_CONST_DICT (
    mp_module_ILI9341_globals,
    ILI9341_globals_table
);

const mp_obj_module_t mp_module_ILI9341 = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_ILI9341_globals
};

//////////////////////////////////////////////////////////////////////////////
// ILI9341 driver implementation
//////////////////////////////////////////////////////////////////////////////

STATIC void disp_spi_init(ILI9341_t *self)
{
    mp_hal_pin_output(self->cs);
    mp_hal_pin_write(self->cs, 1);
    uint32_t state = disable_irq();
    SPI_INIT((uint32_t)self->spihost, (uint32_t)self->cs->pin, self->baudrate, 8, self->mode);
    SPI_GET_CONF((uint32_t)self->spihost, &self->spcmd, &self->spbr);
    enable_irq(state);
}

STATIC void disp_spi_send(ILI9341_t *self, const uint8_t *data, uint16_t length) {
    if (length == 0)
        return; //no need to send anything
    uint32_t state = disable_irq();
    SPI_START_XFER(self->spihost, self->spcmd, self->spbr);
    mp_hal_pin_write(self->cs, 0);
    SPI_TRANSFER(self->spihost, 8, (uint8_t *)0, (uint8_t *)data, (uint32_t)length, 1000);
    mp_hal_pin_write(self->cs, 1);
    enable_irq(state);
}

STATIC void ili9441_send_cmd(ILI9341_t *self, uint8_t cmd) {
    mp_hal_pin_write(self->dc, 0); /*Command mode*/
    disp_spi_send(self, &cmd, 1);
}

STATIC void ili9341_send_data(ILI9341_t *self, const void *data, uint16_t length) {
    mp_hal_pin_write(self->dc, 1); /*Data */
    disp_spi_send(self, data, length);
}

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

STATIC const lcd_init_cmd_t ili_init_cmds[]={
        {0x01, {0}, 0x80},                          /* software reset */
        {0x28, {0}, 0x0},                           /* display off */
        {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},  /* Power control a */
        {0xCF, {0x00, 0xC1, 0X30}, 3},              /* Power control b 83->c1 */
        {0xE8, {0x85, 0x00, 0x78}, 3},              /* driver timing control a 01->00 79->78 */
        {0xEA, {0x00, 0x00}, 2},                    /* driver timing control b */
        {0xED, {0x64, 0x03, 0X12, 0X81}, 4},        /* power on sequence control */
//        {0xF7, {0x20}, 1},                        /* pump ratio control */
        {0xC0, {0x23}, 1},                          /* Power control 1 26->23 */
        {0xC1, {0x10}, 1},                          /* Power control 2 11->10 */
        {0xC5, {0x3e, 0x28}, 2},                    /* VCOM control 1 33->3e,3e->28 */
        {0xC7, {0x86}, 1},                          /* VCOM control 2 be->86 */
        {0x36, {0x48}, 1},                          /* Memory Access Control */
        {0x3A, {0x55}, 1},                          /* Pixel Format Set */
        {0xB1, {0x00, 0x18}, 2},                    /* set frame control 1b->18 */
        {0xB6, {0x08, 0x82, 0x27}, 3},              /* display function control */
//        {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},        /* display function control */
        {0xF2, {0x02}, 1},                          /* enable 3g 08->02 */
        {0x26, {0x01}, 1},                          /* gamma set */
        {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
        {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
        {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
        {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
        {0x2C, {0}, 0},
//        {0xB7, {0x07}, 1},
        {0x11, {0}, 0x80},                          /* sleep out */
        {0x29, {0}, 0x80},                          /* display on */
        {0, {0}, 0xff},
};

STATIC void ili9431_clear(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t col);

STATIC mp_obj_t mp_init_ILI9341(mp_obj_t self_in) {
    ILI9341_t *self = MP_OBJ_TO_PTR(self_in);
    mp_activate_ILI9341(self_in);

    disp_spi_init(self);

    //Initialize non-SPI GPIOs
    mp_hal_pin_output(self->dc);
    mp_hal_pin_output(self->rst);
    if (self->backlight != MP_OBJ_NULL)
        mp_hal_pin_output(self->backlight);

    //Reset the display
    if (self->rst) {
        mp_hal_pin_write(self->rst, 1);
        mp_hal_delay_ms(100);
        mp_hal_pin_write(self->rst, 0);
        mp_hal_delay_ms(400);
        mp_hal_pin_write(self->rst, 1);
        mp_hal_delay_ms(100);
    }

    //Send all the commands
    uint16_t cmd = 0;
    while (ili_init_cmds[cmd].databytes != 0xff) {
        ili9441_send_cmd(self, ili_init_cmds[cmd].cmd);
        ili9341_send_data(self, ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes & 0x1F);
        if (ili_init_cmds[cmd].databytes & 0x80) {
            mp_hal_delay_ms(120);
        }
        cmd++;
    }

    //Enable backlight
    if (self->backlight != MP_OBJ_NULL)
        mp_hal_pin_write(self->backlight, 1);

    //ili9431_clear(0, 0, 239, 319, 0);

    return mp_const_none;
}

STATIC void ili9341_set_addresses(ILI9341_t *self, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    uint8_t data[4];
    /*Column addresses*/
    ili9441_send_cmd(self, 0x2A);
    data[0] = (x1 >> 8) & 0xFF;
    data[1] = x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] = x2 & 0xFF;
    ili9341_send_data(self, data, 4);

    /*Page addresses*/
    ili9441_send_cmd(self, 0x2B);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] = y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] = y2 & 0xFF;
    ili9341_send_data(self, data, 4);

    /*Memory write*/
    ili9441_send_cmd(self, 0x2C);
}

STATIC void ili9431_clear(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t col) {
    ILI9341_t *self = g_ILI9341;
    ili9341_set_addresses(self, x1, y1, x2, y2);
    uint32_t size = (uint32_t)(x2 - x1 + 1) * (uint32_t)(y2 - y1 + 1);
    uint32_t i;
    uint8_t *p = (uint8_t *)&col;
    for (i = 0; i < size; i++) {
        ili9341_send_data(self, (void *)p, 1);
        ili9341_send_data(self, (void *)(p+1), 1);
    }
}

STATIC void ili9431_flush(struct _disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    ILI9341_t *self = g_ILI9341;

    ili9341_set_addresses(self, area->x1, area->y1, area->x2, area->y2);

    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

    /*Byte swapping is required*/
    uint32_t i;
    uint8_t *color_u8 = (uint8_t *)color_p;
    uint8_t color_tmp;
    for (i = 0; i < size * 2; i += 2) {
        color_tmp = color_u8[i + 1];
        color_u8[i + 1] = color_u8[i];
        color_u8[i] = color_tmp;
    }

    ili9341_send_data(self, (void *)color_p, size * 2);

    /*
     while(size > LV_HOR_RES) {

     ili9341_send_data((void*)color_p, LV_HOR_RES * 2);
     //vTaskDelay(10 / portTICK_PERIOD_MS);
     size -= LV_HOR_RES;
     color_p += LV_HOR_RES;
     }

     ili9341_send_data((void*)color_p, size * 2);	*//*Send the remaining data*/

    lv_disp_flush_ready(disp_drv);
}
