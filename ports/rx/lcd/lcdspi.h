/*
 * sLcdSpi.h
 *
 * Copyright (c) 2017 Kentaro Sekimoto
 *
 * This software is released under the MIT License.
 *
 */
#ifndef SLCDSPI_H_
#define SLCDSPI_H_

#include "font.h"
//#include "S1D15G10.h"
//#include "PCF8833.h"
//#include "ST7735.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#endif /* SLCDSPI_H_ */
