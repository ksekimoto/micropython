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

#ifndef LCD_LCDSPI_H_
#define LCD_LCDSPI_H_

#include "font.h"

#ifdef __cplusplus
extern "C" {
#endif

/* LCD Controller */
#define PCF8833     0
#define S1D15G10    1
#define ILI9341     2
#define ILI9340     3
#define ST7735      4
#define ST7789      5

/* LCD Model */
#define NOKIA6100_0     0
#define NOKIA6100_1     1
#define T180            2
#define M022C9340SPI    3
#define RASPI13LCDSPI   4
#define RASPI28LCDSPI   5

// RGB 565 format x2 => RG BR GB 44 44 44 format
// v1: rrrrrggg gggbbbbb
// v2: rrrrrggg gggbbbbb
#define R4G4(v1)        ((uint8_t)(((v1 & 0xf000) >> 8) | ((v1 & 0x07e0) >> 7)))
#define B4R4(v1, v2)    ((uint8_t)(((v1 & 0x1f) << 3) | (v2 >> 12)))
#define G4B4(v2)        ((uint8_t)(((v2 & 0x07e0) >> 3) | ((v2 & 0x1f) >> 1)))

/*
 * LCD SPI pin configuration
 */
enum {n_clk, n_dout, n_din, n_cs, n_reset, n_rs};

typedef struct {
    pin_obj_t *clkPin;
    pin_obj_t *doutPin;
    pin_obj_t *dinPin;
    pin_obj_t *csPin;
    pin_obj_t *resetPin;
    pin_obj_t *rsPin;
} lcdspi_pins_t;

typedef union {
    lcdspi_pins_t pins;
    pin_obj_t  *pina[6];
} lcdspi_pins_u;

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
    const int lcd_info_id;
    void (*lcdspi_init)();
    void (*lcdspi_reset)();
    const lcdspi_ctrl_info_t *lcdspi_ctrl_info;
    const int disp_wx;
    const int disp_wy;
    const int PWX;
    const int PWY;
    const int text_sx;
    const int text_sy;
} lcdspi_info_t;

/*
 * LCD screen information
 */
typedef struct {
    font_t *font;
    uint16_t cx;
    uint16_t cy;
    uint16_t fcol;
    uint16_t bcol;
    int unit_wx;
    int unit_wy;
} lcdspi_screen_t;

typedef struct {
    int spi_id;
    const lcdspi_info_t *lcdspi_info;
    lcdspi_screen_t *lcdspi_screen;
    lcdspi_pins_t *lcdspi_pins;
    uint32_t baud;
    uint16_t spcmd;
    uint8_t spbr;
    uint8_t polarity;
    uint8_t phase;
} lcdspi_t;

void SPISW_Initialize(void);
void SPISW_Reset(void);
void SPISW_LCD_cmd8_0(uint8_t dat);
void SPISW_LCD_dat8_0(uint8_t dat);
void SPISW_LCD_cmd8_1(uint8_t dat);
void SPISW_LCD_dat8_1(uint8_t dat);

void lcdspi_bitbltex565(lcdspi_t *lcdspi, int x, int y, int width, int height, uint16_t *data);
void lcdspi_bitbltex(lcdspi_t *lcdspi, int x, int y, int width, int height, uint16_t *data);

extern const mp_obj_type_t pyb_lcdspi_type;

#ifdef __cplusplus
}
#endif

#endif /* LCD_LCDSPI_H_ */
