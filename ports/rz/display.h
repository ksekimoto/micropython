/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
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

#ifndef PORTS_RZ_DISPLAY_H_
#define PORTS_RZ_DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "font.h"

// #define LCD_BUF_ALIGN    0x20
#define LCD_BUF_ALIGN    0x80   // use 0x80 to share the buffer with camera

#define GFORMAT_YCBCR422 0
#define GFORMAT_RGB565   1
#define GFORMAT_RGB888   2
#define GFORMAT_ARGB8888 3
#define GFORMAT_ARGB4444 4
#define GFORMAT_CLUT8    5
#define GFORMAT_CLUT4    6
#define GFORMAT_CLUT1    7

#define JFORMAT_YCBCR422 0
#define JFORMAT_ARGB8888 1
#define JFORMAT_RGB565   2

#define WRSWA_NON           0
#define WRSWA_8BIT          1
#define WRSWA_16BIT         2
#define WRSWA_16_8BIT       3
#define WRSWA_32BIT         4
#define WRSWA_32_8BIT       5
#define WRSWA_32_16BIT      6
#define WRSWA_32_16_8BIT    7
#define WRSWA_DEF           8

#define WRSWA_NON           0   /*!< Not swapped: 1-2-3-4-5-6-7-8 */
#define WRSWA_8BIT          1   /*!< Swapped in 8-bit units: 2-1-4-3-6-5-8-7 */
#define WRSWA_16BIT         2   /*!< Swapped in 16-bit units: 3-4-1-2-7-8-5-6 */
#define WRSWA_16_8BIT       3   /*!< Swapped in 16-bit units + 8-bit units: 4-3-2-1-8-7-6-5 */
#define WRSWA_32BIT         4   /*!< Swapped in 32-bit units: 5-6-7-8-1-2-3-4 */
#define WRSWA_32_8BIT       5   /*!< Swapped in 32-bit units + 8-bit units: 6-5-8-7-2-1-4-3 */
#define WRSWA_32_16BIT      6   /*!< Swapped in 32-bit units + 16-bit units: 7-8-5-6-3-4-1-2 */
#define WRSWA_32_16_8BIT    7   /*!< Swapped in 32-bit units + 16-bit units + 8-bit units: 8-7-6-5-4-3-2-1 */

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
#define Pink            0xF81F      /* 255, 192, 203 */

typedef struct _display_rect {
    uint16_t vs;          /*!< Vertical start pos       */
    uint16_t vw;          /*!< Vertical width (height)  */
    uint16_t hs;          /*!< Horizontal start pos     */
    uint16_t hw;          /*!< Horizontal width         */
} display_rect_t;

typedef struct _display_console {
    font_t *font;
    uint16_t cx;
    uint16_t cy;
    uint32_t fcol;
    uint32_t bcol;
    uint16_t unit_wx;
    uint16_t unit_wy;
} display_console_t;

typedef struct _display_t {
    bool active;
    uint16_t width;
    uint16_t height;
    uint16_t layer_id;
    uint16_t depth;
    uint16_t stride;
    uint16_t format;
    uint16_t swa;
    uint8_t *buf;
    display_rect_t rect;
    display_console_t console;
} display_t;

void display_clear(display_t *dp, uint32_t color);
void display_pset(display_t *dp, uint16_t x, uint16_t y, uint32_t color);
void display_vline(display_t *dp, uint16_t x, uint16_t y1, uint16_t y2, uint32_t color);
void display_hline(display_t *dp, uint16_t x1, uint16_t y, uint16_t x2, uint32_t color);
void display_box(display_t *dp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void display_box_fill(display_t *dp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void display_line(display_t *dp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void display_circle(display_t *dp, uint16_t x, uint16_t y, uint16_t r, uint32_t color);
void display_circle_fill(display_t *dp, uint16_t x, uint16_t y, uint16_t r, uint32_t color);
void display_write_char_color(display_t *dp, font_t *font, uint8_t c, uint16_t cx, uint16_t cy, uint32_t fgcol, uint32_t bgcol);
void display_write_unicode_color(display_t *dp, font_t *font, uint16_t u, uint16_t cx, uint16_t cy, uint32_t fgcol, uint32_t bgcol);
void display_write_char(display_t *dp, uint8_t c, uint16_t row, uint32_t col);
void display_write_unicode(display_t *dp, uint16_t u, uint16_t row, uint32_t col);
void display_write_formatted_char(display_t *dp, uint8_t ch);
void display_write_formatted_unicode(display_t *dp, uint16_t u);
bool display_init(display_t *dp);
void display_deinit(display_t *dp);
int display_jpeg_save_xy(const char *filename, const char *buf, uint16_t wx, uint16_t wy, uint16_t format);
int display_jpeg_load_xy(const char *filename, const char *buf, uint16_t wx, uint16_t wy, uint16_t format);

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_DISPLAY_H_ */
