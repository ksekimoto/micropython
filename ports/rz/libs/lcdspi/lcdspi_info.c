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

#include <stdio.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "extmod/machine_spi.h"

#include "PCF8833.h"
#include "S1D15G10.h"
#include "ILI9341.h"
#include "ILI9340.h"
#include "ILI9488.h"
#include "ST7735.h"
#include "ST7789.h"
#include "lcdspi.h"
#include "lcdspi_info.h"

#if MICROPY_HW_ENABLE_LCDSPI

#ifdef HIGH
#undef HIGH
#endif
#define HIGH 1
#ifdef LOW
#undef LOW
#endif
#define LOW 0

static void lcdspi_cmd_exec(uint8_t *cmdtbl, uint32_t size);

static void delay_ms(volatile uint32_t n) {
    // mp_hal_delay_ms(n);
    while (n-- > 0) {
        for (int i = 0; i < 30000; i++) {
            __asm__ __volatile__ ("nop");
        }
    }
}

#define DLYMS   (0x00)
#define CMD9    (0x20)
#define DAT9    (0x40)
#define CMD8    (0x60)
#define DAT8    (0x80)

// ===================================================================
// lcd controller
// ===================================================================
/*
 * lcdspi_ctrl_info_t
 *  const uint8_t id;
 *  const uint8_t PASET;
 *  const uint8_t CASET;
 *  const uint8_t RAMWR;
 */

/*
 * lcdspi_lcd_t
 *  char *name;
 *  int lcd_info_id;
 *  void (*lcdspi_init)();
 *  void (*lcdspi_reset)();
 *  int def_baud;
 *  lcdspi_ctrl_info_t *lcdspi_ctrl_info;
 *  int _disp_wx;
 *  int _disp_wy;
 *  int _PWX;
 *  int _PWY;
 *  int _text_sx;
 *  int _text_sy;
 */

/* ********************************************************************* */
/* LCD Controller: PCF8833                                               */
/* LCD: Nokia 6100                                                       */
/* ********************************************************************* */

static void PCF8833_Reset();
static void PCF8833_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_PCF8833 = {
    PCF8833, PCF8833_PASET, PCF8833_CASET, PCF8833_RAMWR
};

static const lcdspi_lcd_t lcdspi_lcd_NOKIA6100_0 = {
    (const char *)"NOKIA6100_0",
    NOKIA6100_0,
    PCF8833_Initialize,
    PCF8833_Reset,
    2000000,
    &lcdspi_ctrl_PCF8833,
    128,
    128,
    2,
    129,
    2,
    129,
    0,      // spi mode 0
    0,      // default madctl
};

static void PCF8833_Reset() {
    delay_ms(100);
    lcdspi_reset_low();
    delay_ms(1000);
    lcdspi_reset_high();
    delay_ms(100);
}

// lcdspi initialize command table
static const uint8_t lcdcmd_PFC8833[] = {
    CMD9 + 0, PCF8833_SLEEPOUT,
    CMD9 + 0, PCF8833_BSTRON,
    DLYMS, 200,
    CMD9 + 1, PCF8833_COLMOD, 0x03,
    CMD9 + 1, PCF8833_MADCTL, 0x00,
    CMD9 + 1, PCF8833_SETCON, 0x35,
    DLYMS, 200,
    CMD9 + 0, PCF8833_DISPON
};
#define PFC8833_CMD_SIZE    (sizeof(lcdcmd_PFC8833) / sizeof(uint8_t))

static void PCF8833_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_PFC8833, PFC8833_CMD_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: S1D15G10                                              */
/* LCD: Nokia 6100                                                       */
/* ********************************************************************* */

static void S1D15G10_Reset();
static void S1D15G10_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_S1D15G10 = {
    S1D15G10, S1D15G10_PASET, S1D15G10_CASET, S1D15G10_RAMWR
};

static const lcdspi_lcd_t lcdspi_lcd_NOKIA6100_1 = {
    (const char *)"NOKIA6100_1",
    NOKIA6100_1,
    S1D15G10_Initialize,
    S1D15G10_Reset,
    2000000,
    &lcdspi_ctrl_S1D15G10,
    128,
    128,
    2,
    129,
    2,
    129,
    0,      // spi mode 0
    0,      // no madctl
};

static void S1D15G10_Reset() {
    delay_ms(100);
    lcdspi_reset_low();
    delay_ms(1000);
    lcdspi_reset_high();
    delay_ms(100);
}

// lcdspi initialize command table
static const uint8_t lcdcmd_S1D15G10[] = {
    DLYMS, 200,
    CMD9 + 3, S1D15G10_DISCTL, 0x0d, 0x20, 0x00,
    CMD9 + 1, S1D15G10_COMSCN, 0x01,
    CMD9 + 0, S1D15G10_OSCON,
    CMD9 + 0, S1D15G10_SLPOUT,
    CMD9 + 1, S1D15G10_PWRCTR, 0x0f,
    CMD9 + 0, S1D15G10_DISNOR,
    CMD9 + 0, S1D15G10_DISINV,
    CMD9 + 0, S1D15G10_PTLOUT,
    CMD9 + 3, S1D15G10_DATCTL, 0x00, 0x00, 0x02,
    CMD9 + 2, S1D15G10_VOLCTR, 0x1c, 0x00,
    CMD9 + 1, S1D15G10_TMPGRD, 0x00,
    DLYMS, 100,
    CMD9 + 0, S1D15G10_DISON,
};
#define S1D15G10_CMD_SIZE   (sizeof(lcdcmd_S1D15G10) / sizeof(uint8_t))

static void S1D15G10_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_S1D15G10, S1D15G10_CMD_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: ILI9341                                               */
/* LCD: xxxxxxxxxx                                                       */
/* ********************************************************************* */

static void ILI9341_Reset();
static void ILI9341_Initialize();
static void ILI9341_22_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_ILI9341 = {
    ILI9341, ILI9341_PASET, ILI9341_CASET, ILI9341_RAMWR
};

static const lcdspi_lcd_t lcdspi_lcd_RASPI28LCDSPI = {
    (const char *)"Adafruit_28_320x240",
    RASPI28LCDSPI,
    ILI9341_Initialize,
    ILI9341_Reset,
    12000000,
    &lcdspi_ctrl_ILI9341,
    240,
    320,
    0,
    239,
    0,
    319,
    0,      // spi mode 0
    0x48,   // default madctl
};

// http://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807
// MSP2807
// MSP2806
// KMRTM24024
static const lcdspi_lcd_t lcdspi_lcd_KMRTM24024SPI = {
    (const char *)"KMRTM24024SPI",
    KMRTM24024SPI,
    ILI9341_Initialize,
    ILI9341_Reset,
    10000000,
    &lcdspi_ctrl_ILI9341,
    240,
    320,
    0,
    239,
    0,
    319,
    0,      // spi mode 0
    0x48,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_AIDEEPEN22SPI = {
    (const char *)"AIDEEPEN22SPI",
    AIDEEPEN22SPI,
    ILI9341_22_Initialize,
    ILI9341_Reset,
    10000000,
    &lcdspi_ctrl_ILI9341,
    240,
    320,
    0,
    239,
    0,
    319,
    0,      // spi mode 0
    0x08,   // default madctl
};

static void ILI9341_Reset() {
    lcdspi_reset_high();
    delay_ms(100);
    lcdspi_reset_low();
    delay_ms(400);
    lcdspi_reset_high();
    delay_ms(100);
}

static const uint8_t lcdcmd_ILI9341[] = {
    CMD8 + 0, 0x01,
    DLYMS, 120,
    CMD8 + 0, 0x28,
    CMD8 + 5, 0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02,
    CMD8 + 3, 0xcf, 0x00, 0xc1, 0x30, /* power control b */
    CMD8 + 3, 0xe8, 0x85, 0x00, 0x78, /* driver timing control a */
    CMD8 + 2, 0xea, 0x00, 0x00, /* driver timing control b */
    CMD8 + 4, 0xed, 0x64, 0x03, 0x12, 0x81, /* power on sequence control */
    CMD8 + 1, 0xc0, 0x23, /* power control 1 */
    CMD8 + 1, 0xc1, 0x10, /* power control 2 */
    CMD8 + 2, 0xc5, 0x3e, 0x28, /* vcom control 1 */
    CMD8 + 1, 0xc7, 0x86, /* vcom control 2 */
    CMD8 + 1, 0x36, 0x48, /* madctl */
    CMD8 + 1, 0x3a, 0x55, /* pixel format 16bit */
    CMD8 + 2, 0xb1, 0x00, 0x18, /* set frame control */
    CMD8 + 3, 0xb6, 0x08, 0x82, 0x27, /* display function control */
    CMD8 + 1, 0xf2, 0x02, /* enable 3g: false */
    CMD8 + 1, 0x26, 0x01, /* gamma set */
    /* positive gamma correction */
    CMD8 + 15, 0xe0, 0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00,
    /* negative gamma correction */
    CMD8 + 15, 0xe1, 0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f,
    CMD8 + 0, 0x11, /* sleep out */
    DLYMS, 120,
    CMD8 + 0, 0x29, /* display on */
};
#define ILI9341_CMD_SIZE   (sizeof(lcdcmd_ILI9341) / sizeof(uint8_t))

static const uint8_t lcdcmd_ILI9341_22[] = {
    CMD8 + 0, 0x01,
    DLYMS, 120,
    CMD8 + 0, 0x28,
    CMD8 + 5, 0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02,
    CMD8 + 3, 0xcf, 0x00, 0xd9, 0x30, /* power control b */
    CMD8 + 3, 0xe8, 0x85, 0x00, 0x7a, /* driver timing control a */
    CMD8 + 2, 0xea, 0x00, 0x00, /* driver timing control b */
    CMD8 + 4, 0xed, 0x64, 0x03, 0x12, 0x81, /* power on sequence control */
    CMD8 + 1, 0xf7, 0x20,
    CMD8 + 1, 0xc0, 0x21, /* power control 1 */
    CMD8 + 1, 0xc1, 0x12, /* power control 2 */
    CMD8 + 2, 0xc5, 0x39, 0x37, /* vcom control 1 */
    CMD8 + 1, 0xc7, 0xab, /* vcom control 2 */
    CMD8 + 1, 0x36, 0x08, /* madctl */
    CMD8 + 1, 0x3a, 0x55, /* pixel format 16bit */
//    CMD8 + 2, 0xb1, 0x00, 0x1b, /* set frame control */
    CMD8 + 2, 0xb1, 0x00, 0x18, /* set frame control */
    CMD8 + 2, 0xb6, 0x08, 0xa2, /* display function control */
    CMD8 + 1, 0xf2, 0x00, /* 3Gamma disable*/
    CMD8 + 1, 0x26, 0x01, /* gamma set */
    CMD8 + 15, 0xe0, 0x0f, 0x23, 0x1f, 0x0b, 0x0e, 0x08, 0x4b, 0xa8, 0x3b, 0x0a, 0x14, 0x06, 0x10, 0x09, 0x00,
    CMD8 + 15, 0xe1, 0x00, 0x1c, 0x20, 0x04, 0x10, 0x08, 0x34, 0x47, 0x44, 0x05, 0x0b, 0x09, 0x2f, 0x36, 0x0f,
    CMD8 + 0, 0x11, /* sleep out */
    DLYMS, 120,
    CMD8 + 0, 0x29, /* display on */
};
#define ILI9341_22_CMD_SIZE   (sizeof(lcdcmd_ILI9341_22) / sizeof(uint8_t))

static void ILI9341_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_ILI9341, ILI9341_CMD_SIZE);
}

static void ILI9341_22_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_ILI9341_22, ILI9341_22_CMD_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: ILI9340                                               */
/* LCD: xxxxxxxxxx                                                       */
/* ********************************************************************* */

static void ILI9340_Reset();
static void ILI9340_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_ILI9340 = {
    ILI9340, ILI9340_PASET, ILI9340_CASET, ILI9340_RAMWR
};

static const lcdspi_lcd_t lcdspi_lcd_M022C9340SPI = {
    (const char *)"AITENDO_M022C9340SPI",
    M022C9340SPI,
    ILI9340_Initialize,
    ILI9340_Reset,
    10000000,
    &lcdspi_ctrl_ILI9340,
    240,
    320,
    0,
    239,
    0,
    319,
    0,      // spi mode 0
    0x48,   // default madctl
};

static void ILI9340_Reset() {
    delay_ms(100);
    lcdspi_reset_low();
    delay_ms(400);
    lcdspi_reset_high();
    delay_ms(100);
}

// lcdspi initialize command table
static const uint8_t lcdcmd_ILI9340[] = {
    CMD8 + 5, 0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02, /* power control a */
    CMD8 + 3, 0xcf, 0x00, 0xc1, 0x30, /* power control b */
    CMD8 + 3, 0xe8, 0x85, 0x00, 0x78, /* driver timing control a */
    CMD8 + 2, 0xea, 0x00, 0x00, /* driver timing control b */
    CMD8 + 4, 0xed, 0x64, 0x03, 0x12, 0x81, /* power on squence control */
    CMD8 + 1, 0xf7, 0x20, /* pump ratio control */
    CMD8 + 1, 0xc0, 0x23, /* power control 1 */
    CMD8 + 1, 0xc1, 0x10, /* power control 2 */
    CMD8 + 2, 0xc5, 0x3e, 0x28, /* vcom control 1 */
    CMD8 + 1, 0xc7, 0x86, /* vcom control 2 */
    CMD8 + 1, 0x36, 0x48, /* madctl */
    CMD8 + 1, 0x3a, 0x55, /* pixel format */
    CMD8 + 2, 0xb1, 0x00, 0x18, /* set frame control */
    CMD8 + 3, 0xb6, 0x08, 0x82, 0x27, /* display function control */
    CMD8 + 1, 0xf2, 0x00, /* enable 3g */
    CMD8 + 1, 0x26, 0x01, /* gamma set */
    CMD8 + 15, 0xe0, 0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00,
    CMD8 + 15, 0xe1, 0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f,
    CMD8 + 0, 0x11, /* sleep out */
    DLYMS, 120,
    CMD8 + 0, 0x2c, /* display on */
};
#define ILI9340_CMD_SIZE   (sizeof(lcdcmd_ILI9340) / sizeof(uint8_t))

static void ILI9340_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_ILI9340, ILI9340_CMD_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: ST7735                                                */
/* ********************************************************************* */

static void ST7735_Reset();
#if OLD_UL018_2P
static void ST7735_Initialize(void);
#endif
#if ST7735B
static void ST7735B_Initialize(void);
#endif
static void ST7735R_G128x160_Initialize(void);
static void ST7735R_R128x160_Initialize(void);
static void ST7735R_R128x160_BGR_Initialize(void);
static void ST7735R_G128x128_Initialize(void);
static void ST7735R_G160x80_Initialize(void);

static const lcdspi_ctrl_info_t lcdspi_ctrl_ST7735 = {
    ST7735, ST7735_PASET, ST7735_CASET, ST7735_RAMWR
};

static void ST7735_Reset() {
    lcdspi_reset_high();
    delay_ms(100);
    lcdspi_reset_low();
    delay_ms(400);
    lcdspi_reset_high();
    delay_ms(100);
}

static void ST77xx_GPIO_INIT(void) {
}

static const lcdspi_lcd_t lcdspi_lcd_T180 = {
    (const char *)"AITENDO_T180",
    T180,
    #if OLD_UL018_2P
    ST7735_Initialize,
    #else
    ST7735R_R128x160_BGR_Initialize,
    #endif
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    128,
    192,
    0,
    127,
    0,
    191,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_ST7735R_G128x160 = {
    (const char *)"ST7735R_G128x160",
    ST7735R_G128x160,
    ST7735R_G128x160_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    128,
    160,
    0,
    127,
    0,
    159,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_ST7735R_R128x160 = {
    (const char *)"ST7735R_R128x160",
    ST7735R_R128x160,
    ST7735R_R128x160_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    128,
    160,
    0,
    127,
    0,
    159,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_ST7735R_G128x128 = {
    (const char *)"ST7735R_G128x128",
    ST7735R_G128x128,
    ST7735R_G128x128_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    128,
    128,
    0,
    127,
    0,
    127,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_ST7735R_G160x80 = {
    (const char *)"ST7735R_G160x80",
    ST7735R_G160x80,
    ST7735R_G160x80_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    128,
    128,
    0,
    127,
    0,
    127,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_KMR18SPI = {
    (const char *)"KMR18SPI",
    ST7735R_R128x160,
    ST7735R_R128x160_BGR_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    128,
    160,
    0,
    127,
    0,
    159,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_WS_18SPI = {
    (const char *)"WS_18SPI",
    WS_18SPI,
    ST7735R_G128x160_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    132,
    162,
    0,
    131,
    0,
    161,
    0,      // spi mode 0
    0x00,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_ST7735R_G130x161 = {
    (const char *)"ST7735R_G130x161",
    ST7735R_G130x161,
    ST7735R_G128x160_Initialize,
    ST7735_Reset,
    2000000,
    &lcdspi_ctrl_ST7735,
    130,
    161,
    2,
    131,
    1,
    161,
    0,      // spi mode 0
    0x00,   // default madctl
};

#if OLD_UL018_2P
// lcdspi initialize command table
static const uint8_t ST7735_cmd[] = {
    CMD8 + 0, 0x01,     // SWRESET
    DLYMS, 255,
    CMD8 + 0, 0x11,     // SLPOUT
    DLYMS, 255,
    CMD8 + 3, 0xb1, 0x01, 0x2c, 0x2d,   // FRMCTR1
    CMD8 + 3, 0xb2, 0x01, 0x2c, 0x2d,   // FRMCTR2
    CMD8 + 6, 0xb3, 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d, // FRMCTR4
    CMD8 + 1, 0xb4, 0x07, /* column inversion */
    CMD8 + 3, 0xc0, 0xa2, 0x02, 0x84,   // PWCTR1
    CMD8 + 1, 0xc1, 0xc5,   // PWCTR2
    CMD8 + 2, 0xc2, 0x0a, 0x00, // PWCTR3
    CMD8 + 2, 0xc3, 0x8a, 0x2a, // PWCTR4
    CMD8 + 2, 0xc4, 0x8a, 0xee, // PWCTR5
    CMD8 + 1, 0xc5, 0x0e,   // VMCTR1
    CMD8 + 1, 0x36, 0xc8, /* mx, my, rgb mode */
    CMD8 + 16, 0xe0, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2b, 0x39, 0x00, 0x01, 0x03, 0x10,
    CMD8 + 16, 0xe1, 0x03, 0x1d, 0x07, 0x06, 0x2e, 0x2c, 0x29, 0x2d, 0x2e, 0x2e, 0x37, 0x3f, 0x00, 0x00, 0x02, 0x10,
    CMD8 + 4, 0x2a, 0x00, 0x02, 0x00, 0x81,
    CMD8 + 4, 0x2b, 0x00, 0x01, 0x00, 0xa0,
    CMD8 + 1, 0x3a, 0x05, /* 4k mode */
//     CMD8 + 0, 0x13,   // NORON
//     DLYMS, 10,
    CMD8 + 0, 0x29,   /* display on */
    DLYMS, 255
};
#define ST7735_CMD_SIZE   (sizeof(ST7735_cmd) / sizeof(uint8_t))
#endif

#if ST7735B
// Init commands for 7735B screens
static const uint8_t ST7735_bcmd[] = {
    CMD8 + 0, 0x01,   // SWRESET
    DLYMS, 50,
    CMD8 + 0, 0x11,   // SLPOUT
    DLYMS, 255,
    CMD8 + 1, 0x3a, 0x05, // COLMOD 4k
    DLYMS, 10,
    CMD8 + 3, 0xb1, 0x00, 0x06, 0x03, // FRMCTR1
    CMD8 + 1, 0x36, 0x08, // MADCTL
    CMD8 + 2, 0xb6, 0x15, 0x02,   // DISSET5
    CMD8 + 1, 0xb4, 0x00, // INVCTR
    CMD8 + 2, 0xc0, 0x02, 0x70,   // PWCTR1
    DLYMS, 10,
    CMD8 + 1, 0xc1, 0x05, // PWCTR2
    CMD8 + 2, 0xc2, 0x01, 0x02,   // PWCTR3
    CMD8 + 2, 0xc5, 0x3c, 0x38,   // VMCTR1
    DLYMS, 10,
    CMD8 + 2, 0xfc, 0x11, 0x15,   // PWCTR6
    CMD8 + 16, 0xe0, 0x09, 0x16, 0x09, 0x20, 0x21, 0x1B, 0x13, 0x19, 0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E,
    CMD8 + 16, 0xe1, 0x0B, 0x14, 0x08, 0x1E, 0x22, 0x1D, 0x18, 0x1E, 0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F,
    DLYMS, 10,
    CMD8 + 4, 0x2a, 0x00, 0x02, 0x00, 0x81,   // XSTART = 2, XEND = 129,
    CMD8 + 4, 0x2b, 0x00, 0x02, 0x00, 0x81,   // YSTART = 2, XEND = 129
    CMD8 + 0, 0x13,   // NORON
    DLYMS, 10,
    CMD8 + 0, 0x29,   // DISPON
    DLYMS, 255
};
#define ST7735_BCMD_SIZE   (sizeof(ST7735_bcmd) / sizeof(uint8_t))
#endif

// 7735R init, part 1 (red or green tab)
static const uint8_t ST7735_rcmd1[] = {
    CMD8 + 0, 0x01,   // SWRESET
    DLYMS, 150,
    CMD8 + 0, 0x11,   // SLPOUT
    DLYMS, 255,
    CMD8 + 3, 0xb1, 0x01, 0x2c, 0x2d, // FRMCTR1
    CMD8 + 3, 0xb2, 0x01, 0x2c, 0x2d, // FRMCTR2
    CMD8 + 6, 0xb3, 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,   // FRMCTR3
    CMD8 + 1, 0xb4, 0x07, // INVCTR
    CMD8 + 3, 0xc0, 0xa2, 0x02, 0x84, // PWCTR1
    CMD8 + 1, 0xc1, 0xc5, // PWCTR2
    CMD8 + 2, 0xc2, 0x0a, 0x00,   // PWCTR3
    CMD8 + 2, 0xc3, 0x8a, 0x2a,   // PWCTR4
    CMD8 + 2, 0xc4, 0x8a, 0xee,   // PWCTR5
    CMD8 + 1, 0xc5, 0x0e, // VMCTR1
    CMD8 + 0, 0x20,   // INVOFF
    CMD8 + 1, 0x36, 0x00, // 0xc8 -> 0x00 MADCTL (RGB)
    CMD8 + 1, 0x3a, 0x05  // COLMOD
};
#define ST7735_RCMD1_SIZE   (sizeof(ST7735_rcmd1) / sizeof(uint8_t))

static const uint8_t BGR[] = {
    CMD8 + 1, 0x36, 0x08, // MADCTL (BGR)
};
#define BGR_SIZE   (sizeof(BGR) / sizeof(uint8_t))

// 7735R init, part 2 (green tab only)
static const uint8_t ST7735_rcmd2green[] = {
    CMD8 + 4, 0x2a, 0x00, 0x02, 0x00, 0x7f + 0x02,  // XSTART = 0, XEND = 127
    CMD8 + 4, 0x2b, 0x00, 0x01, 0x00, 0x9f + 0x01,  // XSTART = 0, XEND = 159
};
#define ST7735_RCMD2GREEN_SIZE  (sizeof(ST7735_rcmd2green) / sizeof(uint8_t))

// 7735R init, part 2 (red tab only)
static const uint8_t ST7735_rcmd2red[] = {
    CMD8 + 4, 0x2a, 0x00, 0x00, 0x00, 0x7f,   // XSTART = 0, XEND = 127
    CMD8 + 4, 0x2b, 0x00, 0x01, 0x00, 0x9f,   // XSTART = 0, XEND = 159
};
#define ST7735_RCMD2RED_SIZE  (sizeof(ST7735_rcmd2red) / sizeof(uint8_t))

// 7735R init, part 2 (green 1.44 tab)
static const uint8_t ST7735_rcmd2green144[] = {
    CMD8 + 4, 0x2a, 0x00, 0x02, 0x00, 0x7f,  // XSTART = 0, XEND = 127
    CMD8 + 4, 0x2b, 0x00, 0x01, 0x00, 0x7f,  // XSTART = 0, XEND = 127
};
#define ST7735_RCMD2GREEN144_SIZE   (sizeof(ST7735_rcmd2green144) / sizeof(uint8_t))

// 7735R init, part 2 (mini 160x80)
static const uint8_t ST7735_rcmd2green160x80[] = {
    CMD8 + 4, 0x2a, 0x00, 0x02, 0x00, 0x7f,  // XSTART = 0, XEND = 127
    CMD8 + 4, 0x2b, 0x00, 0x01, 0x00, 0x7f,  // XSTART = 0, XEND = 127
};
#define ST7735_RCMD2GREEN160x80_SIZE    (sizeof(ST7735_rcmd2green160x80) / sizeof(uint8_t))

// 7735R init, part 3 (red or green tab)
static const uint8_t ST7735_rcmd3[] = {
    CMD8 + 16, 0xe0, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    CMD8 + 16, 0xe1, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    CMD8 + 0, 0x13,   // NORON
    DLYMS, 10,
    CMD8 + 0, 0x29,   // DISPON
    DLYMS, 100
};
#define ST7735_RCMD3_SIZE   (sizeof(ST7735_rcmd3) / sizeof(uint8_t))

#if OLD_UL018_2P
static void ST7735_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_cmd, ST7735_CMD_SIZE);
}
#endif

#if 0
static void ST7735B_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_bcmd, ST7735_BCMD_SIZE);
}
#endif

static void set_BGR(void) {
    lcdspi_cmd_exec((uint8_t *)&BGR, BGR_SIZE);
}

static void ST7735R_G128x160_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd1, ST7735_RCMD1_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd2green, ST7735_RCMD2GREEN_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd3, ST7735_RCMD3_SIZE);
}

static void ST7735R_R128x160_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd1, ST7735_RCMD1_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd2red, ST7735_RCMD2RED_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd3, ST7735_RCMD3_SIZE);
}

static void ST7735R_R128x160_BGR_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd1, ST7735_RCMD1_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd2red, ST7735_RCMD2RED_SIZE);
    set_BGR();
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd3, ST7735_RCMD3_SIZE);
}

static void ST7735R_G128x128_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd1, ST7735_RCMD1_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd2green144, ST7735_RCMD2GREEN144_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd3, ST7735_RCMD3_SIZE);
}

static void ST7735R_G160x80_Initialize(void) {
    ST77xx_GPIO_INIT();
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd1, ST7735_RCMD1_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd2green160x80, ST7735_RCMD2GREEN160x80_SIZE);
    lcdspi_cmd_exec((uint8_t *)&ST7735_rcmd3, ST7735_RCMD3_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: ST7789                                                */
/* ********************************************************************* */

static void ST7789_Reset();
static void ST7789_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_ST7789 = {
    ST7789, ST7789_PASET, ST7789_CASET, ST7789_RAMWR
};

static const lcdspi_lcd_t lcdspi_lcd_RASPI13LCDSPI = {
    (const char *)"RASPI13LCDSPI",
    RASPI13LCDSPI,
    ST7789_Initialize,
    ST7789_Reset,
    24000000,
    &lcdspi_ctrl_ST7789,
    240,
    240,
    0,
    239,
    0,
    239,
    0,      // spi mode 0
    0x10,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_WS_13SPI = {
    (const char *)"WS_13SPI",
    WS_13SPI,
    ST7789_Initialize,
    ST7789_Reset,
    24000000,
    &lcdspi_ctrl_ST7789,
    240,
    240,
    0,
    239,
    0,
    239,
    0,      // spi mode 0
    0x10,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_PIM543 = {
    (const char *)"PIM543",
    PIM543,
    ST7789_Initialize,
    ST7789_Reset,
    24000000,
    &lcdspi_ctrl_ST7789,
    135,
    240,
    52,
    186,
    40,
    279,
    0,      // spi mode 0
    0x10,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_WS_114SPI = {
    (const char *)"WS_114SPI",
    WS_114SPI,
    ST7789_Initialize,
    ST7789_Reset,
    24000000,
    &lcdspi_ctrl_ST7789,
    135,
    240,
    52,
    186,
    40,
    279,
    0,      // spi mode 0
    0x10,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_WS_28SPI = {
    (const char *)"WS_28SPI",
    WS_28SPI,
    ST7789_Initialize,
    ST7789_Reset,
    24000000,
    &lcdspi_ctrl_ST7789,
    240,
    320,
    0,
    239,
    0,
    319,
    0,      // spi mode 0
    0x10,   // default madctl
};

static const lcdspi_lcd_t lcdspi_lcd_GMT130 = {
    (const char *)"GMT130",
    GMT130,
    ST7789_Initialize,
    ST7789_Reset,
    32000000,
    &lcdspi_ctrl_ST7789,
    240,
    240,
    0,
    239,
    0,
    239,
    2,      // spi mode 2
    0x10,   // default madctl
};

static void ST7789_Reset() {
    lcdspi_reset_high();
    delay_ms(100);
    lcdspi_reset_low();
    delay_ms(100);
    lcdspi_reset_high();
    delay_ms(200);
}

static const uint8_t lcdcmd_ST7789[] = {
    CMD8 + 0, 0x01,   // SWRESET
    DLYMS, 150,
    CMD8 + 0, 0x11,   // SLPOUT
    DLYMS, 10,
    CMD8 + 1, 0x3a, 0x55,   // col mod 16 bits/pixel
    DLYMS, 10,
    CMD8 + 1, 0x36, 0x10,   // madctl
    CMD8 + 4, 0x2a, 0, 0, 0, 240,
    CMD8 + 4, 0x2b, 0, 0, 0, 240,
    CMD8 + 0, 0x21,   // INVON
    DLYMS, 10,
    CMD8 + 0, 0x13,         // NORON
    DLYMS, 10,
    CMD8 + 0, 0x29,         // DISPON
    DLYMS, 10,
};
#define ST7789_CMD_SIZE   (sizeof(lcdcmd_ST7789) / sizeof(uint8_t))

static void ST7789_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_ST7789, ST7789_CMD_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: ILI9488                                               */
/* ********************************************************************* */

static void ILI9488_Reset();
static void ILI9488_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_ILI9488 = {
    ILI9488, ILI9488_PASET, ILI9488_CASET, ILI9488_RAMWR
};

static const lcdspi_lcd_t lcdspi_lcd_WS_35SPI = {
    (const char *)"WS_35SPI",
    WS_28SPI,
    ILI9488_Initialize,
    ILI9488_Reset,
    24000000,
    &lcdspi_ctrl_ILI9488,
    320,
    480,
    0,
    319,
    0,
    479,
    0,      // spi mode 0
    0x48,   // default madctl
};

static void ILI9488_Reset() {
    lcdspi_reset_high();
    delay_ms(50);
    lcdspi_reset_low();
    delay_ms(50);
    lcdspi_reset_high();
    delay_ms(150);
}

// lcdspi initialize command table
static const uint8_t lcdcmd_ILI9488[] = {
    CMD8 + 0, 0x21, /*  */
    CMD8 + 1, 0xc2, 0x33, /* Normal mode, increase can change the display quality, while increasing power consumption
        LCD_WriteData(0x33); */
    CMD8 + 3, 0xc5, 0x00, 0x1e, 0x80, /*  */
    CMD8 + 1, 0xb1, 0xb0, /* Sets the frame frequency of full color normal mode, 0XB0 =70HZ, <=0XB0.0xA0=62HZ */
    CMD8 + 1, 0x36, 0x48, /* madctl 2 DOT FRAME MODE,F<=70HZ. */
    CMD8 + 15, 0xe0, 0x00, 0x13, 0x18, 0x04, 0x0f, 0x06, 0x3a, 0x56, 0x4d, 0x03, 0x0a, 0x06, 0x30, 0x3e, 0x0f,
    CMD8 + 15, 0xe1, 0x00, 0x13, 0x18, 0x01, 0x11, 0x06, 0x38, 0x34, 0x4d, 0x06, 0x0d, 0x0b, 0x31, 0x37, 0x0f,
    CMD8 + 1, 0x3a, 0x55, /* col mod 16 bits/pixel */
    CMD8 + 0, 0x11, /* sleep out */
    DLYMS, 120,
    CMD8 + 0, 0x29, /* display on */
};
#define ILI9488_CMD_SIZE   (sizeof(lcdcmd_ILI9488) / sizeof(uint8_t))

static void ILI9488_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_ILI9488, ILI9488_CMD_SIZE);
}

/* ********************************************************************* */
/* LCD Controller: SSD1331                                               */
/* ********************************************************************* */

static void SSD1331_Reset();
static void SSD1331_Initialize();

static const lcdspi_ctrl_info_t lcdspi_ctrl_SSD1331 = {
    SSD1331, 0x15, 0x75, 0
};

static const lcdspi_lcd_t lcdspi_lcd_QT095B = {
    (const char *)"QT095B",
    QT095B,
    SSD1331_Initialize,
    SSD1331_Reset,
    12000000,
    &lcdspi_ctrl_SSD1331,
    96,
    64,
    0,
    95,
    0,
    63,
    3,      // spi mode 0
    0x00,   // default madctl
};

static void SSD1331_Reset() {
    lcdspi_reset_high();
    delay_ms(50);
    lcdspi_reset_low();
    delay_ms(50);
    lcdspi_reset_high();
    delay_ms(150);
}

// lcdspi initialize command table
static const uint8_t lcdcmd_SSD1331[] = {
    CMD8 + 0, 0xae, /* display off */
    CMD8 + 0, 0xa0, /* set remap */
    CMD8 + 0, 0x72, /* RGB color */
//     CMD8 + 0, 0x76, /* BGR color */
    CMD8 + 0, 0xa1, /* start line */
    CMD8 + 0, 0x00, /* 0 */
    CMD8 + 0, 0xa2, /* display offset */
    CMD8 + 0, 0x00, /* 0 */
    CMD8 + 0, 0xa4, /* normal display */
    CMD8 + 0, 0xa8, /* set multiplex */
    CMD8 + 0, 0x3f, /* 1/64 duty */
    CMD8 + 0, 0xad, /* set master */
    CMD8 + 0, 0x8e, /* 0x8e */
    CMD8 + 0, 0xb0, /* power mode */
    CMD8 + 0, 0x0b, /* 0x0b */
    CMD8 + 0, 0xb1, /* pre charge */
    CMD8 + 0, 0x31, /* 0x31 */
    CMD8 + 0, 0xb3, /* clock div */
    CMD8 + 0, 0xf0, /* 0xf0 */
    CMD8 + 0, 0x8a, /* pre charge A */
    CMD8 + 0, 0x64, /* 0x64 */
    CMD8 + 0, 0x8b, /* pre charge B */
    CMD8 + 0, 0x78, /* 0x78 */
    CMD8 + 0, 0x8c, /* pre charge C */
    CMD8 + 0, 0x64, /* 0x64 */
    CMD8 + 0, 0xbb, /* pre charge level */
    CMD8 + 0, 0x64, /* 0x64 */
    CMD8 + 0, 0xbe, /* VCOMH */
    CMD8 + 0, 0x3e, /* 0x3e */
    CMD8 + 0, 0x87, /* master current */
    CMD8 + 0, 0x06, /* 0x06 */
    CMD8 + 0, 0x81, /* contrast A */
    CMD8 + 0, 0x91, /* 0x91 */
    CMD8 + 0, 0x82, /* contrast B */
    CMD8 + 0, 0x50, /* 0x50 */
    CMD8 + 0, 0x83, /* contrast C */
    CMD8 + 0, 0x7d, /* 0x7d */
    CMD8 + 0, 0xaf, /* display on */
    DLYMS, 120,
};
#define SSD1331_CMD_SIZE   (sizeof(lcdcmd_SSD1331) / sizeof(uint8_t))

static void SSD1331_Initialize() {
    lcdspi_cmd_exec((uint8_t *)&lcdcmd_SSD1331, SSD1331_CMD_SIZE);
}

// ===================================================================
// lcdspi lcd info
// ===================================================================

const lcdspi_lcd_t *lcdspi_info[] = {
    &lcdspi_lcd_NOKIA6100_0,        // 0: NOKIA6100_0
    &lcdspi_lcd_NOKIA6100_1,        // 1: NOKIA6100_1
    &lcdspi_lcd_T180,               // 2: T180
    &lcdspi_lcd_M022C9340SPI,       // 3: M022C9340SPI
    &lcdspi_lcd_RASPI13LCDSPI,      // 4: RASPI13LCDSPI
    &lcdspi_lcd_RASPI28LCDSPI,      // 5: RASPI28LCDSPI
    &lcdspi_lcd_KMRTM24024SPI,      // 6: KMRTM24024SPI
    &lcdspi_lcd_KMR18SPI,           // 7: KMR18SPI
    &lcdspi_lcd_ST7735R_G128x160,   // 8: ST7735B
    &lcdspi_lcd_ST7735R_R128x160,   // 9: ST7735R
    &lcdspi_lcd_ST7735R_G128x128,   // 10: ST7735R
    &lcdspi_lcd_ST7735R_G160x80,    // 11: ST7735R
    &lcdspi_lcd_AIDEEPEN22SPI,      // 12: AIDEEPEN22SPI
    &lcdspi_lcd_PIM543,             // 13: PIM543
    &lcdspi_lcd_WS_114SPI,          // 14: WS_114SPI
    &lcdspi_lcd_WS_13SPI,           // 15: WS_13SPI
    &lcdspi_lcd_WS_18SPI,           // 16: WS_18SPI
    &lcdspi_lcd_WS_28SPI,           // 17: WS_284SPI
    &lcdspi_lcd_WS_35SPI,           // 18: WS_135SPI
    &lcdspi_lcd_ST7735R_G130x161,   // 19: ST7735B
    &lcdspi_lcd_GMT130,             // 20: GMT130
    &lcdspi_lcd_QT095B,             // 21: QT095B
};
#define LCDSPI_MOD_SIZE (sizeof(lcdspi_info) / sizeof(lcdspi_lcd_t *))

const uint32_t lcdspi_info_size = LCDSPI_MOD_SIZE;

// ===================================================================
// lcdspi command execute
// ===================================================================

static void lcdspi_cmd_exec(uint8_t *cmdtbl, uint32_t size) {
    uint8_t typ;
    uint8_t num;
    uint8_t cmd;
    uint32_t i = 0;
    while (i < size) {
        typ = cmdtbl[i] & 0xe0;
        num = cmdtbl[i] & 0x1f;
        cmd = cmdtbl[i + 1];
        switch (typ) {
            case DLYMS:
                if (cmd == 0xff) {
                    delay_ms(500);
                } else {
                    delay_ms(cmd);
                }
                break;
            case CMD9:
                lcdspi_spi_write_cmd9(cmd);
                for (uint32_t j = 0; j < num; j++) {
                    lcdspi_spi_write_dat9(cmdtbl[i + 2 + j]);
                }
                break;
            case CMD8:
                lcdspi_spi_write_cmd8(cmd);
                for (uint32_t j = 0; j < num; j++) {
                    lcdspi_spi_write_dat8(cmdtbl[i + 2 + j]);
                }
                break;
        }
        i += (2 + (uint32_t)num);
    }
}

#endif
