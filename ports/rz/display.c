/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
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
#include <stdbool.h>
#include "py/runtime.h"
#include "py/mphal.h"
// #include "dcache-control.h"
#include "mpy_file.h"
#include "mbed_jpeg.h"
#include "rz_buf.h"
#include "display.h"

extern void dcache_clean(void *p_buf, uint32_t size);
extern void dcache_invalid(void *p_buf, uint32_t size);

// typedef unsigned int uint32_t;
// typedef unsigned char uint8_t;

void display_clear(display_t *dp, uint32_t color) {
    uint32_t size = (uint32_t)dp->stride * (uint32_t)dp->height;
    uint8_t *p = (uint8_t *)dp->buf;
    if (dp->format == GFORMAT_RGB565) {
        for (uint32_t i = 0; i < (size / 2); i++) {
            *p++ = (uint8_t)(color & 0xff);
            *p++ = (uint8_t)((color >> 8) & 0xff);
        }
    } else {
        for (uint32_t i = 0; i < (size / 3); i++) {
            *p++ = (uint8_t)((color >> 16) & 0xff);
            *p++ = (uint8_t)((color >> 8) & 0xff);
            *p++ = (uint8_t)(color & 0xff);
        }
    }
    dcache_clean((void *)dp->buf, size);
}

void display_pset(display_t *dp, uint16_t x, uint16_t y, uint32_t color) {
    if ((x >= dp->height) || (y >= dp->width)) {
        return;
    }
    uint32_t i = (uint32_t)dp->stride * (uint32_t)y + (uint32_t)x * (uint32_t)dp->depth;
    uint8_t *p = (uint8_t *)(dp->buf + i);
    if (dp->format == GFORMAT_RGB565) {
        *p = (uint8_t)(color & 0xff);
        *(p + 1) = (uint8_t)((color >> 8) & 0xff);
        dcache_clean((void *)p, 2);
    } else {
        *p = (uint8_t)((color >> 16) & 0xff);
        *(p + 1) = (uint8_t)((color >> 8) & 0xff);
        *(p + 2) = (uint8_t)(color & 0xff);
        dcache_clean((void *)p, 3);
    }
}

static void swap_uint32(uint16_t *i, uint16_t *j) {
    uint16_t t;
    t = *j;
    *j = *i;
    *i = t;
}

static inline int16_t _abs(int16_t x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}

static inline int16_t sign(int16_t x) {
    if (x < 0) {
        return -1;
    } else {
        return 1;
    }
}

void display_vline(display_t *dp, uint16_t x, uint16_t y1, uint16_t y2, uint32_t color) {
    if (y1 > y2) {
        swap_uint32(&y1, &y2);
    }
    for (uint16_t j = 0; j < (y2 - y1 + 1); j++) {
        display_pset(dp, x, y1 + j, color);
    }
}

void display_hline(display_t *dp, uint16_t x1, uint16_t y, uint16_t x2, uint32_t color) {
    if (x1 > x2) {
        swap_uint32(&x1, &x2);
    }
    for (uint16_t j = 0; j < (x2 - x1 + 1); j++) {
        display_pset(dp, x1 + j, y, color);
    }
}

void display_box(display_t *dp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    display_hline(dp, x1, y1, x2, color);
    display_vline(dp, x2, y1, y2, color);
    display_hline(dp, x1, y2, x2, color);
    display_vline(dp, x1, y1, y2, color);
}

void display_box_fill(display_t *dp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    if (x1 > x2) {
        swap_uint32(&x1, &x2);
    }
    if (y1 > y2) {
        swap_uint32(&y1, &y2);
    }
    for (uint16_t j = 0; j < (y2 - y1 + 1); j++) {
        display_hline(dp, x1, y1 + j, x2, color);
    }
}

void display_line(display_t *dp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;
    int16_t s1;
    int16_t s2;
    int16_t temp;
    int16_t interchange;
    int16_t e;
    int16_t i;

    x = x1;
    y = y1;
    dx = _abs((int16_t)x2 - (int16_t)x1);
    dy = _abs((int16_t)y2 - (int16_t)y1);
    s1 = sign((int16_t)x2 - (int16_t)x1);
    s2 = sign((int16_t)y2 - (int16_t)y1);

    if (dy > dx) {
        temp = dx;
        dx = dy;
        dy = temp;
        interchange = 1;
    } else {
        interchange = 0;
    }
    e = 2 * dy - dx;
    for (i = 0; i <= dx; i++) {
        display_pset(dp, x, y, color);
        while (e >= 0) {
            if (interchange == 1) {
                x += s1;
            } else {
                y += s2;
            }
            e -= 2 * dx;
        }
        if (interchange == 1) {
            y += s2;
        } else {
            x += s1;
        }
        e += 2 * dy;
    }
}

static void display_circle_sub(display_t *dp, uint16_t x, uint16_t y, uint16_t r, uint32_t color, bool fill) {
    int16_t xi = 0;
    int16_t yi = (int32_t)r;
    int16_t di = 2 * (1 - (int32_t)r);
    int16_t limit = 0;
    int16_t delta1;
    int16_t delta2;

    while (1) {
        if (fill) {
            display_hline(dp, (int16_t)x - xi, (int16_t)y - yi, (int16_t)x + xi, color);
            display_hline(dp, (int16_t)x - xi, (int16_t)y + yi, (int16_t)x + xi, color);
        } else {
            display_pset(dp, (int16_t)x + xi, (int16_t)y + yi, color);
            display_pset(dp, (int16_t)x - xi, (int16_t)y + yi, color);
            display_pset(dp, (int16_t)x + xi, (int16_t)y - yi, color);
            display_pset(dp, (int16_t)x - xi, (int16_t)y - yi, color);
        }
        if (yi <= limit) {
            goto circle4;
        }
        if (di < 0) {
            goto circle2;
        }
        if (di > 0) {
            goto circle3;
        }
        // if (di == 0)
        goto circle20;
    circle2: delta1 = 2 * di + 2 * yi - 1;
        if (delta1 <= 0) {
            goto circle10;
        }
        // if (delta > 0)
        goto circle20;
    circle3: delta2 = 2 * di - 2 * xi - 1;
        if (delta2 <= 0) {
            goto circle20;
        }
        // if (delta2 > 0)
        goto circle30;
    circle10: xi += 1;
        di += (2 * xi + 1);
        continue;   // goto circle1;
    circle20: xi += 1;
        yi -= 1;
        di += (2 * xi - 2 * yi + 2);
        continue;   // goto circle1;
    circle30: yi -= 1;
        di -= (2 * yi - 1);
        continue;   // goto circle1;
    }
circle4: return;
}

void display_circle(display_t *dp, uint16_t x, uint16_t y, uint16_t r, uint32_t color) {
    display_circle_sub(dp, x, y, r, color, false);
}

void display_circle_fill(display_t *dp, uint16_t x, uint16_t y, uint16_t r, uint32_t color) {
    display_circle_sub(dp, x, y, r, color, true);
}

void display_write_char_color(display_t *dp, font_t *font, uint8_t c, uint16_t cx, uint16_t cy, uint32_t fgcol, uint32_t bgcol) {
    uint16_t ux, uy;
    uint16_t wx, wy;
    uint16_t sx, sy;
    uint16_t off;
    uint32_t color;
    uint8_t *data;

    if (font == (font_t *)NULL) {
        return;
    }
    if (c >= 0x80) {
        c = 0;
    }
    data = (uint8_t *)font_fontData(font, (int)(c & 0x00ff));
    ux = (uint16_t)font_fontUnitX(font);
    uy = (uint16_t)font_fontUnitY(font);
    wx = (uint16_t)font_fontWidth(font, (int)(c & 0x00ff));
    wy = (uint16_t)font_fontHeight(font, (int)(c & 0x00ff));
    off = 0;
    sx = ux * cx;
    sy = uy * cy;
    for (uint16_t y = 0; y < wy; y++) {
        for (uint16_t x = 0; x < wx; x++) {
            if (x == 8) {
                off++;
            }
            if (data[y] & (0x80 >> x)) {
                color = fgcol;
            } else {
                color = bgcol;
            }
            display_pset(dp, sx + x, sy + y, color);
        }
        off++;
    }
}

void display_write_unicode_color(display_t *dp, font_t *font, uint16_t u, uint16_t cx, uint16_t cy, uint32_t fgcol, uint32_t bgcol) {
    uint16_t ux, uy;
    uint16_t wx, wy;
    uint16_t sx, sy;
    uint32_t off;
    uint32_t color;
    uint8_t *data;

    if (font == (font_t *)NULL) {
        return;
    }
    data = (uint8_t *)font_fontData(font, (int)u);
    ux = (uint16_t)font_fontUnitX(font);
    uy = (uint16_t)font_fontUnitY(font);
    wx = (uint16_t)font_fontWidth(font, (int)u);
    wy = (uint16_t)font_fontHeight(font, (int)u);
    off = 0;
    sx = ux * cx;
    sy = uy * cy;
    for (uint16_t y = 0; y < wy; y++) {
        for (uint16_t x = 0; x < wx; x++) {
            if (x == 8) {
                off++;
            }
            if (data[off] & (0x80 >> (x & 0x7))) {
                color = fgcol;
            } else {
                color = bgcol;
            }
            display_pset(dp, sx + x, sy + y, color);
        }
        off++;
    }
}

void display_write_char(display_t *dp, uint8_t c, uint16_t row, uint32_t col) {
    display_write_char_color(dp, dp->console.font, c, row, col, dp->console.fcol, dp->console.bcol);
}

void display_write_unicode(display_t *dp, uint16_t u, uint16_t row, uint32_t col) {
    display_write_unicode_color(dp, dp->console.font, u, row, col, dp->console.fcol, dp->console.bcol);
}

void display_write_formatted_char(display_t *dp, uint8_t ch) {
    display_console_t *console = &dp->console;
    uint16_t cx = console->cx;
    uint16_t cy = console->cy;
    uint16_t unit_x = (uint16_t)font_fontUnitX(console->font);
    uint16_t unit_y = (uint16_t)font_fontUnitY(console->font);
    uint16_t disp_wx = dp->height;
    uint16_t disp_wy = dp->width;
    if (ch == 0xc) {
        cx = 0;
        cy = 0;
    } else if (ch == '\n') {
        cy++;
        if (cy == disp_wy / unit_y) {
            cy = 0;
        }
    } else if (ch == '\r') {
        cx = 0;
    } else {
        display_write_char_color(dp, console->font, ch, cx, cy, console->fcol, console->bcol);
        cx++;
        if (cx == disp_wx / unit_x) {
            cx = 0;
            cy++;
            if (cy == disp_wy / unit_y) {
                cy = 0;
            }
        }
    }
    console->cx = (uint16_t)cx;
    console->cy = (uint16_t)cy;
}

void display_write_formatted_unicode(display_t *dp, uint16_t u) {
    display_console_t *console = &dp->console;
    uint16_t cx = console->cx;
    uint16_t cy = console->cy;
    uint16_t unit_x = (uint16_t)font_fontUnitX(console->font);
    uint16_t unit_y = (uint16_t)font_fontUnitY(console->font);
    uint16_t disp_wx = dp->height;
    uint16_t disp_wy = dp->width;
    if ((char)u == 0xc) {
        cx = 0;
        cy = 0;
    } else if ((char)u == '\n') {
        cy++;
        if (cy == disp_wy / unit_y) {
            cy = 0;
        }
    } else if ((char)u == '\r') {
        cx = 0;
    } else {
        display_write_unicode_color(dp, console->font, u, cx, cy, console->fcol, console->bcol);
        if (u < 0x100) {
            cx++;
        } else {
            cx += 2;
        }
        if (cx >= disp_wx / unit_x) {
            cx = 0;
            cy++;
            if (cy == disp_wy / unit_y) {
                cy = 0;
            }
        }
    }
    console->cx = (uint16_t)cx;
    console->cy = (uint16_t)cy;
}

bool display_init(display_t *dp) {
    if (dp->rect.hw == 0) {
        dp->rect.hw = dp->width;
    }
    if (dp->rect.vw == 0) {
        dp->rect.vw = dp->height;
    }
    switch (dp->format) {
        case GFORMAT_YCBCR422:
            dp->depth = 2;
            break;
        case GFORMAT_RGB565:
            dp->depth = 2;
            break;
        case GFORMAT_RGB888:
            dp->depth = 3;
            break;
        case GFORMAT_ARGB8888:
            dp->depth = 4;
            break;
        case GFORMAT_ARGB4444:
            dp->depth = 2;
            break;
        case GFORMAT_CLUT8:
            dp->depth = 1;
            break;
        case GFORMAT_CLUT4:
            dp->depth = 1;
            break;
        case GFORMAT_CLUT1:
            dp->depth = 1;
            break;
        default:
            dp->depth = 2;
            break;
    }
    if (dp->stride == 0) {
        dp->stride = ((dp->width * dp->depth + 31u) & ~31u);
    }
    if (dp->buf == (uint8_t *)NULL) {
        dp->buf = (uint8_t *)rz_malloc((size_t)(dp->stride * dp->height) + LCD_BUF_ALIGN);
        if (dp->buf == (uint8_t *)NULL) {
            return false;
        }
        dp->buf = (uint8_t *)(((uint32_t)dp->buf + LCD_BUF_ALIGN) & ~(LCD_BUF_ALIGN - 1));
    }
    return true;
}

void display_deinit(display_t *dp) {
    if (dp->buf) {
        rz_free(dp->buf);
    }
}

int display_jpeg_save_xy(const char *filename, const char *buf, uint16_t wx, uint16_t wy, uint16_t format) {
    uint32_t size = (uint32_t)wx * (uint32_t)wy * 2;
    uint32_t written;
    char *jpeg_buf;
    int err = 0;

    mp_obj_t file = mf_open(filename);
    written = size;
    err = mbed_jpeg_encode(buf, wx, wy, (char **)&jpeg_buf, &written, format);
    if ((err == 0) && (written >= 0) && (written < size)) {
        written = mf_write(file, (void *)jpeg_buf, written);
    }
    mf_close(file);
    return err;
}

int display_jpeg_load_xy(const char *filename, const char *buf, uint16_t wx, uint16_t wy, uint16_t format) {
    uint32_t readed;
    char *jpeg_buf;
    int err = 0;

    mp_obj_t file = mf_open(filename);
    readed = (uint32_t)mf_size(file);
    jpeg_buf = (char *)rz_malloc(readed);
    if (jpeg_buf) {
        if (readed > 0) {
            readed = mf_read(file, (void *)jpeg_buf, readed);
        }
        err = mbed_jpeg_decode(buf, wx, wy, (char *)jpeg_buf, readed, format);
        rz_free(jpeg_buf);
    }
    mf_close(file);
    return err;
}
