/*
 * Copyright (c) 2020, Kentaro Sekimoto
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

#ifndef LCD_LCDSPI_H_
#define LCD_LCDSPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "font.h"

#ifdef PIN_NONE
#undef PIN_NONE
#endif
#define PIN_NONE 0xffff

/* LCD SPI Channel */
#define SW_LCDSPI_CH (0xffffffff)

/* LCD Controller */
#define PCF8833  0
#define S1D15G10 1
#define ILI9341  2
#define ILI9340  3
#define ST7735   4
#define ST7789   5
#define ILI9488  6

/* LCD Model */
#define NOKIA6100_0   0
#define NOKIA6100_1   1
#define T180          2         // T18SPI - ST7735 - RDDID: 0x5c, 0x89, 0xf0
#define M022C9340SPI  3
#define RASPI13LCDSPI 4
#define RASPI28LCDSPI 5
#define KMRTM24024SPI 6
#define KMR18SPI      7         // KMR18SPI - ST7735S - RDDID: 0x7c, 0x89, 0xf0
#define ST7735R_G128x160    8
#define ST7735R_R128x160    9
#define ST7735R_G128x128    10
#define ST7735R_G160x80     11
#define ROBOT_LCD   8
#define AIDEEPEN22SPI 12
#define PIM543 13
#define WS_114SPI   14          // Not Tested
#define WS_13SPI    15          // Not Tested
#define WS_18SPI    16
#define WS_28SPI    17
#define WS_35SPI    18
#define ST7735R_G130x161    19

// RGB 565 format x2 => RG BR GB 44 44 44 format
// v1: rrrrrggg gggbbbbb
// v2: rrrrrggg gggbbbbb
#define R4G4(v1)     ((uint8_t)(((v1 & 0xf000) >> 8) | ((v1 & 0x07e0) >> 7)))
#define B4R4(v1, v2) ((uint8_t)(((v1 & 0x1f) << 3) | (v2 >> 12)))
#define G4B4(v2)     ((uint8_t)(((v2 & 0x07e0) >> 3) | ((v2 & 0x1f) >> 1)))

#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */
#define Pink            0xF81F

/*
 * LCD SPI pin configuration
 */
typedef struct {
    uint32_t pin_clk;
    uint32_t pin_dout;
    uint32_t pin_din;
    uint32_t pin_cs;
    uint32_t pin_reset;
    uint32_t pin_rs;
} lcdspi_pins_t;

/*
 * LCD Controller information
 */
typedef struct {
    const uint8_t id;
    const uint8_t PASET;
    const uint8_t CASET;
    const uint8_t RAMWR;
} lcdspi_ctrl_info_t;

/*
 * LCD module information
 */
typedef struct {
    const char *name;
    const uint32_t lcd_info_id;
    void (*lcdspi_init)();
    void (*lcdspi_reset)();
    const uint32_t def_baud;
    const lcdspi_ctrl_info_t *ctrl_info;
    const uint16_t width;
    const uint16_t height;
    const uint16_t sx;
    const uint16_t ex;
    const uint16_t sy;
    const uint16_t ey;
} lcdspi_lcd_t;

/*
 * LCD screen information
 */
typedef struct {
    font_t *font;
    uint16_t cx;
    uint16_t cy;
    uint16_t fcol;
    uint16_t bcol;
    uint32_t unit_wx;
    uint32_t unit_wy;
    bool scroll;
    uint16_t dy;    // scroll start
} lcdspi_screen_t;

typedef void (*lcdspi_gpio_output_t)(uint32_t pin_id);
typedef void (*lcdspi_gpio_input_t)(uint32_t pin_id);
typedef void (*lcdspi_gpio_write_t)(uint32_t pin_id, bool v);
typedef bool (*lcdspi_gpio_read_t)(uint32_t pin_id);
typedef void (*lcdspi_spi_init_t)(void);
typedef void (*lcdspi_spi_transfer_t)(size_t len, const uint8_t *src, uint8_t *dest);

/*
 * LCD SPI information
 */
typedef struct {
    uint32_t spi_ch;
    lcdspi_lcd_t *lcd;
    lcdspi_screen_t *screen;
    lcdspi_pins_t *pins;
    lcdspi_gpio_output_t gpio_output;
    lcdspi_gpio_input_t gpio_input;
    lcdspi_gpio_write_t gpio_write;
    lcdspi_gpio_read_t gpio_read;
    lcdspi_spi_init_t spi_init;
    lcdspi_spi_transfer_t spi_transfer;
    uint32_t baud;
    uint16_t spcmd;
    uint8_t spbr;
    uint8_t polarity;
    uint8_t phase;
    uint32_t did1;
    uint32_t did2;
    uint8_t id1_1;
    uint8_t id2_1;
    uint8_t id3_1;
    uint8_t id1_2;
    uint8_t id2_2;
    uint8_t id3_2;
    uint8_t id1_3;
    uint8_t id2_3;
    uint8_t id3_3;
    uint32_t ili93xx_id_spisw;
    uint32_t ili93xx_id_spihw;
} lcdspi_t;

void lcdspi_reset_high(void);
void lcdspi_reset_low(void);
void lcdspi_spisw_init(void);
void lcdspi_spisw_write(uint8_t dat);
uint8_t lcdspi_spisw_xfer(uint8_t dat);
void lcdspi_spi_write(uint8_t dat);
uint8_t lcdspi_spi_xfer(uint8_t dat);
void lcdspi_spi_transfer(uint8_t *dst, const uint8_t *src, uint32_t count);
void lcdspi_spi_write_cmd9(uint8_t dat);
void lcdspi_spi_write_dat9(uint8_t dat);
void lcdspi_spi_write_cmd8(uint8_t dat);
void lcdspi_spi_write_dat8(uint8_t dat);
void lcdspi_spi_write_dat8_2(uint16_t dat);
void lcdspi_spi_write_dat16(uint16_t dat);
void lcdspi_spi_write_dat16_n(uint32_t size, uint16_t v);
void lcdspi_spi_write_buf_n(uint8_t *buf, uint32_t size);
uint8_t lcdspi_spisw_read_reg(uint8_t addr, uint8_t idx);
uint8_t lcdspi_spihw_read_reg(uint8_t addr, uint8_t idx);
uint8_t ILI93xx_spisw_read_reg(uint8_t addr, uint8_t idx);
uint8_t ILI93xx_spihw_read_reg(uint8_t addr, uint8_t idx);

void lcdspi_set_spi_ch(lcdspi_t *lcdspi, uint32_t spi_ch);
void lcdspi_set_pins(lcdspi_t *lcdspi, lcdspi_pins_t *lcdspi_pins);
void lcdspi_set_lcd(lcdspi_t *lcdspi, uint32_t lcd_id);
void lcdspi_set_screen(lcdspi_t *lcdspi, lcdspi_screen_t *lcdspi_screen);
void lcdspi_spi_init(lcdspi_t *lcdspi, bool spw_hw_mode);
void lcdspi_box_fill(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col);
void lcdspi_clear(lcdspi_t *lcdspi, uint16_t col);
void lcdspi_scroll(lcdspi_t *lcdspi, uint32_t dy);
void lcdspi_screen_init(lcdspi_screen_t *lcdspi_screen);
void lcdspi_init(lcdspi_t *lcdspi, lcdspi_screen_t *screen, lcdspi_pins_t *pins, uint32_t lcd_id, uint32_t spi_ch);
void lcdspi_deinit(lcdspi_t *lcdspi);
void lcdspi_bitbltex565(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint16_t *data);
void lcdspi_bitbltex(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint16_t *data);
void lcdspi_pset(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint16_t col);
void lcdspi_box(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col);
void lcdspi_line(lcdspi_t *lcdspi, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint16_t col);
void lcdspi_circle(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t r, uint16_t col);
void lcdspi_circle_fill(lcdspi_t *lcdspi, uint32_t x, uint32_t y, uint32_t r, uint16_t col);
void lcdspi_write_char_color_xy(lcdspi_t *lcdspi, unsigned char c, uint32_t x, uint32_t y, uint16_t fgcol, uint16_t bgcol);
void lcdspi_write_char_color(lcdspi_t *lcdspi, unsigned char c, uint32_t cx, uint32_t cy, uint16_t fgcol, uint16_t bgcol);
void lcdspi_write_unicode_color_xy(lcdspi_t *lcdspi, unsigned short u, uint32_t x, uint32_t y, uint16_t fgcol, uint16_t bgcol);
void lcdspi_write_unicode_color(lcdspi_t *lcdspi, unsigned short u, uint32_t cx, uint32_t cy, uint16_t fgcol, uint16_t bgcol);
void lcdspi_write_char_xy(lcdspi_t *lcdspi, unsigned char c, uint32_t x, uint32_t y);
void lcdspi_write_char(lcdspi_t *lcdspi, unsigned char c, uint32_t row, uint32_t col);
void lcdspi_write_unicode_xy(lcdspi_t *lcdspi, unsigned short u, uint32_t x, uint32_t y);
void lcdspi_write_unicode(lcdspi_t *lcdspi, unsigned short u, uint32_t row, uint32_t col);
void lcdspi_write_formatted_char(lcdspi_t *lcdspi, unsigned char ch);
void lcdspi_write_formatted_unicode(lcdspi_t *lcdspi, unsigned short u);
void lcdspi_set_font(lcdspi_t *lcdspi, font_t *font);
font_t *lcdspi_get_font(lcdspi_t *lcdspi);
unsigned short cnvUtf8ToUnicode(unsigned char *str, uint32_t *size);
int lcdspi_disp_bmp_file(lcdspi_t *lcdspi, int x, int y, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* LCD_LCDSPI_H_ */
