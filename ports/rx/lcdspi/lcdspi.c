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

#include <stdio.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "ff.h"

#include "PCF8833.h"
#include "S1D15G10.h"
#include "ILI9341.h"
#include "ILI9340.h"
#include "ST7735.h"
#include "ST7789.h"
#include "font.h"
#if defined(RX65N)
#include "rx65n_gpio.h"
#include "rx65n_spi.h"
#endif
#if defined(RZA2M)
#include "rza2m_gpio.h"
#include "rza2m_spi.h"
#endif
#include "jpeg.h"
#include "lcdspi.h"

#if MICROPY_PY_PYB_LCDSPI

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

//#define TEST_LCDSPI
#ifdef TEST_LCDSPI
#include "sLcdSpiTest.h"
#endif

#if defined(USE_DBG_PRINT)
#define DEBUG_LCDSPI
#endif

//#define LCDSPI_OPTIMIZE

#define HIGH    1
#define LOW     0

static void PCF8833_Reset();
static void PCF8833_Initialize();
static void S1D15G10_Reset();
static void S1D15G10_Initialize();
static void ILI9341_Reset();
static void ILI9341_Initialize();
static void ILI9340_Reset();
static void ILI9340_Initialize();
static void ST7735_Reset();
static void ST7735_Initialize();
static void ST7789_Reset();
static void ST7789_Initialize();

static volatile int g_spi_id = 0;

#if defined(GRCITRUS)
static uint8_t _clkPin = PC5;
static uint8_t _doutPin = PC6;
static uint8_t _dinPin = PC4;
static uint8_t _csPin = PC7;
static uint8_t _resetPin = P52;
static uint8_t _rsPin = P50;

static lcdspi_pins_t lcdspi_pins_def = {
    (pin_obj_t *)&pin_PC5_obj,   /* clk */
    (pin_obj_t *)&pin_PC6_obj,   /* mosi */
    (pin_obj_t *)&pin_PC7_obj,   /* miso */
    (pin_obj_t *)&pin_PC4_obj,   /* cs */
    (pin_obj_t *)&pin_P52_obj,   /* reset */
    (pin_obj_t *)&pin_P50_obj,   /* rs */
};
#endif
#if defined(GRSAKURA)
/* Aitendo M0_1114_basic_shield */
static uint8_t _clkPin = P33;
static uint8_t _doutPin = P32;
static uint8_t _dinPin = P22;
static uint8_t _csPin = P24;
static uint8_t _resetPin = P25;
static uint8_t _rsPin = P23;

/*
 *  pin_obj_t *clkPin;
 *  pin_obj_t *doutPin;
 *  pin_obj_t *dinPin;
 *  pin_obj_t *csPin;
 *  pin_obj_t *resetPin;
 *  pin_obj_t *rsPin;
 */
static lcdspi_pins_t lcdspi_pins_def = {
    (pin_obj_t *)&pin_P33_obj,   /* clk */
    (pin_obj_t *)&pin_P32_obj,   /* mosi */
    (pin_obj_t *)&pin_P22_obj,   /* miso */
    (pin_obj_t *)&pin_P24_obj,   /* cs */
    (pin_obj_t *)&pin_P25_obj,   /* reset */
    (pin_obj_t *)&pin_P23_obj,   /* rs */
};
#endif
#if defined(GRROSE)
static uint8_t _clkPin = PE5;
static uint8_t _doutPin = PE6;
static uint8_t _dinPin = PE7;
static uint8_t _csPin = PE4;
static uint8_t _resetPin = P26;
static uint8_t _rsPin = P30;

static lcdspi_pins_t lcdspi_pins_def = {
    (pin_obj_t *)&pin_PE5_obj,   /* clk */
    (pin_obj_t *)&pin_PE6_obj,   /* mosi */
    (pin_obj_t *)&pin_PE4_obj,   /* miso */
    (pin_obj_t *)&pin_PE4_obj,   /* cs */
    (pin_obj_t *)&pin_P26_obj,   /* reset */
    (pin_obj_t *)&pin_P30_obj,   /* rs */
};
#endif
#if defined(GRMANGO)
static uint32_t _clkPin = P87;
static uint32_t _doutPin = P86;
static uint32_t _dinPin = P85;
static uint32_t _csPin = P84;
static uint32_t _resetPin = P45;
static uint32_t _rsPin = PH6;

static lcdspi_pins_t lcdspi_pins_def = {
    (pin_obj_t *)&pin_P87_obj,   /* clk */
    (pin_obj_t *)&pin_P86_obj,   /* mosi */
    (pin_obj_t *)&pin_P85_obj,   /* miso */
    (pin_obj_t *)&pin_P84_obj,   /* cs */
    (pin_obj_t *)&pin_P45_obj,   /* reset */
    (pin_obj_t *)&pin_PH6_obj,   /* rs */
};
#endif

/*
 * const uint8_t id;
 * const uint8_t PASET;
 * const uint8_t CASET;
 * const uint8_t RAMWR;
 */
static const lcdspi_ctrl_info_t lcdspi_ctrl_PCF8833 = {
    PCF8833, PCF8833_PASET, PCF8833_CASET, PCF8833_RAMWR
};

static const lcdspi_ctrl_info_t lcdspi_ctrl_S1D15G10 = {
    S1D15G10, S1D15G10_PASET, S1D15G10_CASET, S1D15G10_RAMWR
};

static const lcdspi_ctrl_info_t lcdspi_ctrl_ST7735 = {
    ST7735, ST7735_PASET, ST7735_CASET, ST7735_RAMWR
};

static const lcdspi_ctrl_info_t lcdspi_ctrl_ILI9341 = {
    ILI9341, ILI9341_PASET, ILI9341_CASET, ILI9341_RAMWR
};

static const lcdspi_ctrl_info_t lcdspi_ctrl_ILI9340 = {
    ILI9340, ILI9340_PASET, ILI9340_CASET, ILI9340_RAMWR
};

static const lcdspi_ctrl_info_t lcdspi_ctrl_ST7789 = {
    ST7789, ST7789_PASET, ST7789_CASET, ST7789_RAMWR
};

/*
 *  char *name;
 *  int lcd_info_id;
 *  void (*lcdspi_init)();
 *  void (*lcdspi_reset)();
 *  lcdspi_ctrl_info_t *lcdspi_ctrl_info;
 *  int _disp_wx;
 *  int _disp_wy;
 *  int _PWX;
 *  int _PWY;
 *  int _text_sx;
 *  int _text_sy;
 */

static const lcdspi_info_t lcdspi_info_NOKIA6100_0 = {
    (const char *)"NOKIA6100_0",
    NOKIA6100_0,
    PCF8833_Initialize,
    PCF8833_Reset,
    &lcdspi_ctrl_PCF8833,
    128,
    128,
    132,
    132,
    2,
    2,
};

static const lcdspi_info_t lcdspi_info_NOKIA6100_1 = {
    (const char *)"NOKIA6100_1",
    NOKIA6100_1,
    S1D15G10_Initialize,
    S1D15G10_Reset,
    &lcdspi_ctrl_S1D15G10,
    128,
    128,
    132,
    132,
    2,
    2,
};

static const lcdspi_info_t lcdspi_info_T180 = {
    (const char *)"AITENDO_T180",
    T180,
    ST7735_Initialize,
    ST7735_Reset,
    &lcdspi_ctrl_ST7735,
    128,
    192,
    128,
    192,
    0,
    0,
};

static const lcdspi_info_t lcdspi_info_M022C9340SPI = {
    (const char *)"AITENDO_M022C9340SPI",
    M022C9340SPI,
    ILI9340_Initialize,
    ILI9340_Reset,
    &lcdspi_ctrl_ILI9340,
    240,
    320,
    240,
    320,
    0,
    0,
};

static const lcdspi_info_t lcdspi_info_13LCDSPI = {
    (const char *)"AITENDO_M022C9340SPI",
    RASPI13LCDSPI,
    ST7789_Initialize,
    ST7789_Reset,
    &lcdspi_ctrl_ST7789,
    240,
    240,
    240,
    240,
    0,
    0,
};

static const lcdspi_info_t lcdspi_info_28LCDSPI = {
    (const char *)"Adafruit_28_320x240",
    RASPI28LCDSPI,
    ILI9341_Initialize,
    ILI9341_Reset,
    &lcdspi_ctrl_ILI9341,
    240,
    320,
    240,
    320,
    0,
    0,
};

#if 0
static const lcdspi_info_t *lcdspi_info_all[] = {
    &lcdspi_info_NOKIA6100_0,
    &lcdspi_info_NOKIA6100_1,
    &lcdspi_info_T180,
    &lcdspi_info_M022C9340SPI,
    $lcdspi_info_13LCDSPI,
};
#endif


/*
 *  int spi_id;
 *  const lcdspi_info_t *lcdspi_info;
 *  lcdspi_screen_t *lcdspi_screen;
 *  lcdspi_pins_t *lcdspi_pins;
 *  uint32_t baud;
 *  uint16_t spcmd;
 *  uint8_t spbr;
 */

static lcdspi_t lcdspi_all[] = {
    {-1, &lcdspi_info_NOKIA6100_0, 0, 0, 2000000, 0, 0},
    {-1, &lcdspi_info_NOKIA6100_1, 0, 0, 2000000, 0, 0},
    {-1, &lcdspi_info_T180, 0, 0, 2000000, 0, 0},
    {-1, &lcdspi_info_M022C9340SPI, 0, 0, 4000000, 0, 0},
    {-1, &lcdspi_info_13LCDSPI, 0, 0, 24000000, 0, 0},
    {-1, &lcdspi_info_28LCDSPI, 0, 0, 12000000, 0, 0},
};

typedef struct _pyb_lcdspi_obj_t {
    mp_obj_base_t base;
    mp_uint_t lcdspi_id;
    lcdspi_t *lcdspi;
    lcdspi_screen_t lcdspi_screen;
    lcdspi_pins_t   lcdspi_pins;
} pyb_lcdspi_obj_t;

static void delay_ms(volatile uint32_t n) {
#if defined(GRMANGO)
    wait_ms(n);
#else
    mdelay(n);
#endif
}

static void SPIHW_SetPinMode(void) {
#if 0
    MPC.PWPR.BYTE = 0x00u;
    MPC.PWPR.BYTE = 0x40u;
    MPC.PC5PFS.BIT.PSEL = 0x0d;
    MPC.PC6PFS.BIT.PSEL = 0x0d;
    MPC.PC7PFS.BIT.PSEL = 0x0d;
    PORTC.PMR.BIT.B5 = 1;
    PORTC.PMR.BIT.B6 = 1;
    PORTC.PMR.BIT.B7 = 1;
#endif
}

static void SPISW_SetPinMode(void) {
#if 0
    MPC.PWPR.BYTE = 0x00u;
    MPC.PWPR.BYTE = 0x40u;
    MPC.PC5PFS.BIT.PSEL = 0x00;
    MPC.PC6PFS.BIT.PSEL = 0x00;
    MPC.PC7PFS.BIT.PSEL = 0x00;
    PORTC.PMR.BIT.B5 = 0;
    PORTC.PMR.BIT.B6 = 0;
    PORTC.PMR.BIT.B7 = 0;
#endif
}

void SPISW_Initialize() {
    GPIO_SET_OUTPUT(_csPin);
    GPIO_SET_OUTPUT(_resetPin);
    GPIO_SET_OUTPUT(_rsPin);
    GPIO_SET_OUTPUT(_clkPin);
    GPIO_SET_OUTPUT(_doutPin);
    GPIO_SET_INPUT(_dinPin);
    GPIO_WRITE(_csPin, LOW);
}

#if defined(LCDSPI_OPTIMIZE)
void SPISW_Write(uint8_t dat) {
    uint8_t i = 8;
    uint32_t port_clk = GPIO_PORT(_clkPin);
    uint32_t bit_clk = GPIO_BIT(_clkPin);
    uint32_t port_dout = GPIO_PORT(_doutPin);
    uint32_t bit_dout = GPIO_BIT(_doutPin);
    uint32_t irq_state = disable_irq();
    while (i-- > 0) {
#if defined(LCDSPI_OPTIMIZE)
        if (dat & 0x80) {
            bit_set((uint8_t *)_PPODR(port_dout), bit_dout);
        } else {
            bit_clr((uint8_t *)_PPODR(port_dout), bit_dout);
        }
        bit_clr((uint8_t *)_PPODR(port_clk), bit_clk);
        bit_set((uint8_t *)_PPODR(port_clk), bit_clk);
        dat <<= 1;
#else
        uint8_t value = (dat & 0x80) ? 1 : 0;
        GPIO_WRITE(_doutPin, value);
        GPIO_WRITE(_clkPin, LOW);
        GPIO_WRITE(_clkPin, HIGH);
        dat <<= 1;
#endif
    }
    enable_irq(irq_state);
}
#else
void SPISW_Write(uint8_t dat) {
    uint8_t i = 8;
    uint8_t value;
    uint32_t irq_state = disable_irq();
    while (i-- > 0) {
        value = (dat & 0x80) ? 1 : 0;
        GPIO_WRITE(_doutPin, value);
        GPIO_WRITE(_clkPin, LOW);
        GPIO_WRITE(_clkPin, HIGH);
        dat <<= 1;
    }
    enable_irq(irq_state);
}
#endif

void SPI_Write(uint8_t dat) {
    if (g_spi_id >= 0) {
#if 0
        SPIHW_SetPinMode(void)
#endif
        // ToDo
        SPI_WRITE_BYTE((uint32_t)g_spi_id, dat);
#if 0
        SPISW_SetPinMode(void)
#endif
    } else {
        SPISW_Write(dat);
    }
}

void SPISW_LCD_cmd8_0(uint8_t dat) {
    // Enter command mode: SDATA=LOW at rising edge of 1st SCLK
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_doutPin, LOW);
    GPIO_WRITE(_clkPin, LOW);
    GPIO_WRITE(_clkPin, HIGH);
    SPI_Write(dat);
    GPIO_WRITE(_csPin, HIGH);
}

void SPISW_LCD_dat8_0(uint8_t dat) {
    // Enter data mode: SDATA=HIGH at rising edge of 1st SCLK
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_doutPin, HIGH);
    GPIO_WRITE(_clkPin, LOW);
    GPIO_WRITE(_clkPin, HIGH);
    SPI_Write(dat);
    GPIO_WRITE(_csPin, HIGH);
}

void SPISW_LCD_cmd8_1(uint8_t dat) {
    // Enter command mode: RS=LOW at rising edge of 1st SCLK
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_rsPin, LOW);
    SPI_Write(dat);
    GPIO_WRITE(_csPin, HIGH);
}

void SPISW_LCD_dat8_1(uint8_t dat) {
    // Enter data mode: RS=HIGH at rising edge of 1st SCLK
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_rsPin, HIGH);
    SPI_Write(dat);
    GPIO_WRITE(_csPin, HIGH);
}

void SPI_cmd_data_1(uint8_t cmd, uint8_t *data, uint32_t data_size) {
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_rsPin, LOW);
    SPI_Write(cmd);
    GPIO_WRITE(_rsPin, HIGH);
    while (data_size-- > 0) {
        SPI_Write(*data++);
    }
    GPIO_WRITE(_csPin, HIGH);
}

/* ********************************************************************* */
/* LCD Controller: PCF8833                                               */
/* LCD: Nokia 6100                                                       */
/* ********************************************************************* */

void lcdspi_spi_init(int spi_id, lcdspi_t *lcdspi) {
    if (spi_id >= 0) {
        SPIHW_SetPinMode();
        GPIO_SET_OUTPUT(_csPin);
        GPIO_SET_OUTPUT(_resetPin);
        GPIO_SET_OUTPUT(_rsPin);
        // ToDo
        SPI_INIT(spi_id, lcdspi->lcdspi_pins->csPin->pin, lcdspi->baud, 8, (lcdspi->phase << 1 | lcdspi->polarity) & 0x3);
        SPI_GET_CONF(spi_id, &lcdspi->spcmd, &lcdspi->spbr);
    } else {
        SPISW_SetPinMode();
        SPISW_Initialize();
    }
}

static void lcdspi_spi_start_xfer(int spi_id, lcdspi_t *lcdspi) {
    if (spi_id >= 0) {
        uint16_t spcmd = lcdspi->spcmd;
        uint8_t spbr = lcdspi->spbr;
        SPI_START_XFER(spi_id, spcmd, spbr);
    } else {
        SPISW_SetPinMode();
    }
}

static void lcdspi_spi_end_xfer(int spi_id, lcdspi_t *lcdspi) {
    if (spi_id >= 0) {
        SPI_END_XFER(spi_id);
    } else {
        SPIHW_SetPinMode();
    }
}

/* ********************************************************************* */
/* LCD Controller: PCF8833                                               */
/* LCD: Nokia 6100                                                       */
/* ********************************************************************* */

static void PCF8833_Reset() {
    delay_ms(100);
    GPIO_WRITE(_resetPin, LOW);
    delay_ms(1000);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(100);
}

static void PCF8833_Initialize() {
    //SPI_Initialize();
    PCF8833_Reset();
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_doutPin, LOW);
    GPIO_WRITE(_clkPin, HIGH);

    SPISW_LCD_cmd8_0(PCF8833_SLEEPOUT);
    SPISW_LCD_cmd8_0(PCF8833_BSTRON);
    delay_ms(200);
    SPISW_LCD_cmd8_0(PCF8833_COLMOD);
    SPISW_LCD_dat8_0(0x03);
    SPISW_LCD_cmd8_0(PCF8833_MADCTL);
    SPISW_LCD_dat8_0(0x00);      // 0xc0 MirrorX, MirrorY
    SPISW_LCD_cmd8_0(PCF8833_SETCON);
    SPISW_LCD_dat8_0(0x35);
    delay_ms(200);
    SPISW_LCD_cmd8_0(PCF8833_DISPON);
}

/* ********************************************************************* */
/* LCD Controller: S1D15G10                                              */
/* LCD: Nokia 6100                                                       */
/* ********************************************************************* */

static void S1D15G10_Reset() {
    delay_ms(100);
    GPIO_WRITE(_resetPin, LOW);
    delay_ms(1000);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(100);
}

static void S1D15G10_Initialize() {
    //SPI_Initialize();
    S1D15G10_Reset();
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_doutPin, LOW);
    GPIO_WRITE(_clkPin, HIGH);

    delay_ms(200);
    SPISW_LCD_cmd8_0(S1D15G10_DISCTL);	// Display Control
    SPISW_LCD_dat8_0(0x0d);	// 0x00 = 2 divisions, switching period=8 (default)
    SPISW_LCD_dat8_0(0x20);				// 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
    SPISW_LCD_dat8_0(0x00);				// 0x00 = no inversely highlighted lines
    SPISW_LCD_cmd8_0(S1D15G10_COMSCN);	// common output scan direction
    SPISW_LCD_dat8_0(0x01);				// 0x01 = Scan 1->80, 160<-81
    SPISW_LCD_cmd8_0(S1D15G10_OSCON);// oscillators on and get out of sleep mode.
    SPISW_LCD_cmd8_0(S1D15G10_SLPOUT);	// sleep out
    SPISW_LCD_cmd8_0(S1D15G10_PWRCTR);// reference voltage regulator on, circuit voltage follower on, BOOST ON
    SPISW_LCD_dat8_0(0xf);	// everything on, no external reference resistors
    SPISW_LCD_cmd8_0(S1D15G10_DISNOR);	// display mode
    SPISW_LCD_cmd8_0(S1D15G10_DISINV);	// display mode
    SPISW_LCD_cmd8_0(S1D15G10_PTLOUT);	// Partial area off
    SPISW_LCD_cmd8_0(S1D15G10_DATCTL);// The DATCTL command selects the display mode (8-bit or 12-bit).
    SPISW_LCD_dat8_0(0x00);	// 0x01 = page address inverted, col address normal, address scan in col direction
    SPISW_LCD_dat8_0(0x00);				// 0x00 = RGB sequence (default value)
    SPISW_LCD_dat8_0(0x02);	// 0x02 = Grayscale -> 16 (selects 12-bit color, type A)
    SPISW_LCD_cmd8_0(S1D15G10_VOLCTR);// The contrast is set by the Electronic Volume Command (VOLCTR).
    SPISW_LCD_dat8_0(28);// 32 volume value (adjust this setting for your display 0 .. 63)
    SPISW_LCD_dat8_0(0);		// 3 resistance ratio (determined by experiment)
    SPISW_LCD_cmd8_0(S1D15G10_TMPGRD);	// Temprature gradient
    SPISW_LCD_dat8_0(0);				// default value
    delay_ms(100);
    SPISW_LCD_dat8_0(0);
    SPISW_LCD_cmd8_0(S1D15G10_DISON);	// display on
}

/* ********************************************************************* */
/* LCD Controller: ST7735                                                */
/* ********************************************************************* */

static void ST7735_Reset() {
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(100);
    GPIO_WRITE(_resetPin, LOW);
    delay_ms(400);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(100);
}

static void ST7735_Initialize() {
    //SPI_Initialize();
    ST7735_Reset();
    GPIO_WRITE(_doutPin, LOW);
    GPIO_WRITE(_csPin, HIGH);
    GPIO_WRITE(_clkPin, HIGH);

    SPISW_LCD_cmd8_1(0x11);
    delay_ms(50);
    SPISW_LCD_cmd8_1(0xB1);
    SPISW_LCD_dat8_1(0x01);
    SPISW_LCD_dat8_1(0x2C);
    SPISW_LCD_dat8_1(0x2D);
    SPISW_LCD_cmd8_1(0xB2);
    SPISW_LCD_dat8_1(0x01);
    SPISW_LCD_dat8_1(0x2C);
    SPISW_LCD_dat8_1(0x2D);
    SPISW_LCD_cmd8_1(0xB3);
    SPISW_LCD_dat8_1(0x01);
    SPISW_LCD_dat8_1(0x2C);
    SPISW_LCD_dat8_1(0x2D);
    SPISW_LCD_dat8_1(0x01);
    SPISW_LCD_dat8_1(0x2C);
    SPISW_LCD_dat8_1(0x2D);
    SPISW_LCD_cmd8_1(0xB4);//Column inversion
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_cmd8_1(0xC0);
    SPISW_LCD_dat8_1(0xA2);
    SPISW_LCD_dat8_1(0x02);
    SPISW_LCD_dat8_1(0x84);
    SPISW_LCD_cmd8_1(0xC1);
    SPISW_LCD_dat8_1(0xC5);
    SPISW_LCD_cmd8_1(0xC2);
    SPISW_LCD_dat8_1(0x0A);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_cmd8_1(0xC3);
    SPISW_LCD_dat8_1(0x8A);
    SPISW_LCD_dat8_1(0x2A);
    SPISW_LCD_cmd8_1(0xC4);
    SPISW_LCD_dat8_1(0x8A);
    SPISW_LCD_dat8_1(0xEE);

    SPISW_LCD_cmd8_1(0xC5);//VCOM
    SPISW_LCD_dat8_1(0x0E);
    SPISW_LCD_cmd8_1(0x36);//MX, MY, RGB mode
    SPISW_LCD_dat8_1(0xC8);
    SPISW_LCD_cmd8_1(0xe0);
    SPISW_LCD_dat8_1(0x02);
    SPISW_LCD_dat8_1(0x1c);
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_dat8_1(0x12);
    SPISW_LCD_dat8_1(0x37);
    SPISW_LCD_dat8_1(0x32);
    SPISW_LCD_dat8_1(0x29);
    SPISW_LCD_dat8_1(0x2d);
    SPISW_LCD_dat8_1(0x29);
    SPISW_LCD_dat8_1(0x25);
    SPISW_LCD_dat8_1(0x2b);
    SPISW_LCD_dat8_1(0x39);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x01);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x10);
    SPISW_LCD_cmd8_1(0xe1);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x1d);
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_dat8_1(0x06);
    SPISW_LCD_dat8_1(0x2e);
    SPISW_LCD_dat8_1(0x2c);
    SPISW_LCD_dat8_1(0x29);
    SPISW_LCD_dat8_1(0x2d);
    SPISW_LCD_dat8_1(0x2e);
    SPISW_LCD_dat8_1(0x2e);
    SPISW_LCD_dat8_1(0x37);
    SPISW_LCD_dat8_1(0x3f);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x02);
    SPISW_LCD_dat8_1(0x10);
    SPISW_LCD_cmd8_1(0x2A);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x02);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x81);
    SPISW_LCD_cmd8_1(0x2B);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x01);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0xA0);
    SPISW_LCD_cmd8_1(0x3A);   // 4k mode
    SPISW_LCD_dat8_1(0x05);
    //SPISW_LCD_cmd(0x3A); // 65k mode
    //SPISW_LCD_dat(0x05);
    SPISW_LCD_cmd8_1(0x29);   //Display on
}

/* ********************************************************************* */
/* LCD Controller: ILI9341                                               */
/* LCD: xxxxxxxxxx                                                       */
/* ********************************************************************* */

static void ILI9341_Reset() {
    delay_ms(100);
    GPIO_WRITE(_resetPin, LOW);
    delay_ms(400);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(100);
}

static void ILI9341_Initialize() {
    //SPI_Initialize();
    ILI9341_Reset();

    SPISW_LCD_cmd8_1(0x01); /* software reset */
    delay_ms(10);
    SPISW_LCD_cmd8_1(0x28); /* display off */

    SPISW_LCD_cmd8_1(0xcb); /* power control a */
    SPISW_LCD_dat8_1(0x39);
    SPISW_LCD_dat8_1(0x2c);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x34);
    SPISW_LCD_dat8_1(0x02);

    SPISW_LCD_cmd8_1(0xcf); /* power control b */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0xc1);
    SPISW_LCD_dat8_1(0x30);

    SPISW_LCD_cmd8_1(0xe8); /* driver timing control a */
    SPISW_LCD_dat8_1(0x85);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x78);

    SPISW_LCD_cmd8_1(0xea); /* driver timing control b */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x00);

    SPISW_LCD_cmd8_1(0xed); /* power on sequence control */
    SPISW_LCD_dat8_1(0x64);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x12);
    SPISW_LCD_dat8_1(0x81);

//    SPISW_LCD_cmd8_1(0xf7); /* pump ratio control */
//    SPISW_LCD_dat8_1(0x20);

    SPISW_LCD_cmd8_1(0xc0); /* power control 1 */
    SPISW_LCD_dat8_1(0x23);

    SPISW_LCD_cmd8_1(0xc1); /* power control 2 */
    SPISW_LCD_dat8_1(0x10);

    SPISW_LCD_cmd8_1(0xc5); /* vcom control 1 */
    SPISW_LCD_dat8_1(0x3e);
    SPISW_LCD_dat8_1(0x28);

    SPISW_LCD_cmd8_1(0xc7); /* vcom control 2 */
    SPISW_LCD_dat8_1(0x86);

    SPISW_LCD_cmd8_1(0x36); /* madctl */
    SPISW_LCD_dat8_1(0x48);

    SPISW_LCD_cmd8_1(0x3a); /* pixel format */
    SPISW_LCD_dat8_1(0x55); /* 16bit */

    SPISW_LCD_cmd8_1(0xb1); /* set frame control */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x18); /* value */

    SPISW_LCD_cmd8_1(0xb6); /* display function control */
    SPISW_LCD_dat8_1(0x08);
    SPISW_LCD_dat8_1(0x82);
    SPISW_LCD_dat8_1(0x27);

    SPISW_LCD_cmd8_1(0xf2); /* enable 3g */
    SPISW_LCD_dat8_1(0x02); /* false */

    SPISW_LCD_cmd8_1(0x26); /* gamma set */
    SPISW_LCD_dat8_1(0x01);

    SPISW_LCD_cmd8_1(0xe0); /* positive gamma correction */
    SPISW_LCD_dat8_1(0x0f);
    SPISW_LCD_dat8_1(0x31);
    SPISW_LCD_dat8_1(0x2b);
    SPISW_LCD_dat8_1(0x0c);
    SPISW_LCD_dat8_1(0x0e);
    SPISW_LCD_dat8_1(0x08);
    SPISW_LCD_dat8_1(0x4e);
    SPISW_LCD_dat8_1(0xf1);
    SPISW_LCD_dat8_1(0x37);
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_dat8_1(0x10);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x0e);
    SPISW_LCD_dat8_1(0x09);
    SPISW_LCD_dat8_1(0x00);

    SPISW_LCD_cmd8_1(0xe1); /* negative gamma correction */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x0e);
    SPISW_LCD_dat8_1(0x14);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x11);
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_dat8_1(0x31);
    SPISW_LCD_dat8_1(0xc1);
    SPISW_LCD_dat8_1(0x48);
    SPISW_LCD_dat8_1(0x08);
    SPISW_LCD_dat8_1(0x0f);
    SPISW_LCD_dat8_1(0x0c);
    SPISW_LCD_dat8_1(0x31);
    SPISW_LCD_dat8_1(0x36);
    SPISW_LCD_dat8_1(0x0f);

    SPISW_LCD_cmd8_1(0x11); /* sleep out */
    delay_ms(120);

    SPISW_LCD_cmd8_1(0x29); /* display on */
}

/* ********************************************************************* */
/* LCD Controller: ILI9340                                               */
/* LCD: xxxxxxxxxx                                                       */
/* ********************************************************************* */

static void ILI9340_Reset() {
    delay_ms(100);
    GPIO_WRITE(_resetPin, LOW);
    delay_ms(400);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(100);
}

static void ILI9340_Initialize() {
    //SPI_Initialize();
    ILI9340_Reset();

    SPISW_LCD_cmd8_1(0xcb); /* power control a */
    SPISW_LCD_dat8_1(0x39);
    SPISW_LCD_dat8_1(0x2c);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x34);
    SPISW_LCD_dat8_1(0x02);

    SPISW_LCD_cmd8_1(0xcf); /* power control b */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0xc1);
    SPISW_LCD_dat8_1(0x30);

    SPISW_LCD_cmd8_1(0xe8); /* driver timing control a */
    SPISW_LCD_dat8_1(0x85);
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x78);

    SPISW_LCD_cmd8_1(0xea); /* driver timing control b */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x00);

    SPISW_LCD_cmd8_1(0xed); /* power on sequence control */
    SPISW_LCD_dat8_1(0x64);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x12);
    SPISW_LCD_dat8_1(0x81);

    SPISW_LCD_cmd8_1(0xf7); /* pump ratio control */
    SPISW_LCD_dat8_1(0x20);

    SPISW_LCD_cmd8_1(0xc0); /* power control 1 */
    SPISW_LCD_dat8_1(0x23);

    SPISW_LCD_cmd8_1(0xc1); /* power control 2 */
    SPISW_LCD_dat8_1(0x10);

    SPISW_LCD_cmd8_1(0xc5); /* vcom control 1 */
    SPISW_LCD_dat8_1(0x3e);
    SPISW_LCD_dat8_1(0x28);

    SPISW_LCD_cmd8_1(0xc7); /* vcom control 2 */
    SPISW_LCD_dat8_1(0x86);

    SPISW_LCD_cmd8_1(0x36); /* madctl */
    SPISW_LCD_dat8_1(0x48);

    SPISW_LCD_cmd8_1(0x3a); /* pixel format */
    SPISW_LCD_dat8_1(0x55);

    SPISW_LCD_cmd8_1(0xb1); /* set frame control */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x18);

    SPISW_LCD_cmd8_1(0xb6); /* display function control */
    SPISW_LCD_dat8_1(0x08);
    SPISW_LCD_dat8_1(0x82);
    SPISW_LCD_dat8_1(0x27);

    SPISW_LCD_cmd8_1(0xf2); /* enable 3g */
    SPISW_LCD_dat8_1(0x00);

    SPISW_LCD_cmd8_1(0x26); /* gamma set */
    SPISW_LCD_dat8_1(0x01);

    SPISW_LCD_cmd8_1(0xe0); /* positive gamma correction */
    SPISW_LCD_dat8_1(0x0f);
    SPISW_LCD_dat8_1(0x31);
    SPISW_LCD_dat8_1(0x2b);
    SPISW_LCD_dat8_1(0x0c);
    SPISW_LCD_dat8_1(0x0e);
    SPISW_LCD_dat8_1(0x08);
    SPISW_LCD_dat8_1(0x4e);
    SPISW_LCD_dat8_1(0xf1);
    SPISW_LCD_dat8_1(0x37);
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_dat8_1(0x10);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x0e);
    SPISW_LCD_dat8_1(0x09);
    SPISW_LCD_dat8_1(0x00);

    SPISW_LCD_cmd8_1(0xe1); /* negative gamma correction */
    SPISW_LCD_dat8_1(0x00);
    SPISW_LCD_dat8_1(0x0e);
    SPISW_LCD_dat8_1(0x14);
    SPISW_LCD_dat8_1(0x03);
    SPISW_LCD_dat8_1(0x11);
    SPISW_LCD_dat8_1(0x07);
    SPISW_LCD_dat8_1(0x31);
    SPISW_LCD_dat8_1(0xc1);
    SPISW_LCD_dat8_1(0x48);
    SPISW_LCD_dat8_1(0x08);
    SPISW_LCD_dat8_1(0x0f);
    SPISW_LCD_dat8_1(0x0c);
    SPISW_LCD_dat8_1(0x31);
    SPISW_LCD_dat8_1(0x36);
    SPISW_LCD_dat8_1(0x0f);

    SPISW_LCD_cmd8_1(0x11); /* sleep out */
    delay_ms(120);

    //SPISW_LCD_cmd8_1(0x29);
    SPISW_LCD_cmd8_1(0x2c); /* display on? */
}

/* ********************************************************************* */
/* LCD Controller: ST7789                                                */
/* ********************************************************************* */

static void ST7789_Reset() {
    GPIO_WRITE(_csPin, LOW);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(50);
    GPIO_WRITE(_resetPin, LOW);
    delay_ms(50);
    GPIO_WRITE(_resetPin, HIGH);
    delay_ms(150);
    GPIO_WRITE(_csPin, HIGH);
}

static void ST7789_Initialize() {
    ST7789_Reset();

#if 1
    /* soft reset */
    SPISW_LCD_cmd8_1(0x01);
    delay_ms(50);
    /* sleep out */
    SPISW_LCD_cmd8_1(0x11);
    /* color mode */
    SPISW_LCD_cmd8_1(0x3a);
    SPISW_LCD_dat8_1(ST7789_COLORMODE_65K | ST7789_COLORMODE_16BIT);
    delay_ms(50);
    /* MADCTL */
    SPISW_LCD_cmd8_1(0x36);
    SPISW_LCD_dat8_1(0x10);
    /* inversion mode on */
    SPISW_LCD_cmd8_1(0x21);
    delay_ms(10);
    /* norm on */
    SPISW_LCD_cmd8_1(0x13);
    delay_ms(10);
    /* disp on */
    SPISW_LCD_cmd8_1(0x29);
    delay_ms(500);
#else
    /* soft reset */
    SPI_cmd_data_1(0x01, &data, 0);
    delay_ms(150);
    /* sleep out */
    SPI_cmd_data_1(0x11, &data, 0);
    /* color mode */
    data = (ST7789_COLORMODE_65K | ST7789_COLORMODE_16BIT);
    SPI_cmd_data_1(0x3a, &data, 1);
    delay_ms(50);
    /* MADCTL */
    data = 0x10;
    SPI_cmd_data_1(0x36, &data, 1);
    /* inversion mode on */
    SPISW_LCD_cmd8_1(0x21);
    delay_ms(10);
    /* norm on */
    SPISW_LCD_cmd8_1(0x13);
    delay_ms(10);
    /* disp on */
    SPISW_LCD_cmd8_1(0x29);
    delay_ms(1000);
#endif
}

/*
 * common functions
 */

static void ILI9340_addrset(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
#if 0
    uint8_t data[4];
    data[0] = (uint8_t)(x1 >> 8);
    data[1] = (uint8_t)(x1 & 0xff);
    data[2] = (uint8_t)(x2 >> 8);
    data[3] = (uint8_t)(x2 & 0xff);
    SPI_cmd_data_1(ILI9340_CASET, &data[0], 4);
    data[0] = (uint8_t)(y1 >> 8);
    data[1] = (uint8_t)(y1 & 0xff);
    data[2] = (uint8_t)(y2 >> 8);
    data[3] = (uint8_t)(y2 & 0xff);
    SPI_cmd_data_1(ILI9340_PASET, &data[0], 4);
    SPI_cmd_data_1(ILI9340_RAMWR, &data[0], 0);
#else
    SPISW_LCD_cmd8_1(ILI9340_CASET);
    SPISW_LCD_dat8_1((uint8_t)(x1 >> 8));
    SPISW_LCD_dat8_1((uint8_t)(x1 & 0xff));
    SPISW_LCD_dat8_1((uint8_t)(x2 >> 8));
    SPISW_LCD_dat8_1((uint8_t)(x2 & 0xff));
    SPISW_LCD_cmd8_1(ILI9340_PASET);
    SPISW_LCD_dat8_1((uint8_t)(y1 >> 8));
    SPISW_LCD_dat8_1((uint8_t)(y1 & 0xff));
    SPISW_LCD_dat8_1((uint8_t)(y2 >> 8));
    SPISW_LCD_dat8_1((uint8_t)(y2 & 0xff));
    SPISW_LCD_cmd8_1(ILI9340_RAMWR);
#endif
}

void lcdspi_clear(lcdspi_t *lcdspi) {
    int x, y;
    int spi_id = lcdspi->spi_id;
    uint8_t PASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcdspi_info->lcdspi_ctrl_info->RAMWR;
    int PWX = lcdspi->lcdspi_info->PWX;
    int PWY = lcdspi->lcdspi_info->PWY;
    int lcd_ctrl_id = lcdspi->lcdspi_info->lcdspi_ctrl_info->id;
    lcdspi_spi_start_xfer(spi_id, lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        SPISW_LCD_cmd8_0(PASET);
        SPISW_LCD_dat8_0(0);
        SPISW_LCD_dat8_0((uint8_t)(PWX - 1));
        SPISW_LCD_cmd8_0(CASET);
        SPISW_LCD_dat8_0(0);
        SPISW_LCD_dat8_0((uint8_t)(PWY - 1));
        SPISW_LCD_cmd8_0(RAMWR);
        for (x = 0; x < ((PWX * PWY) / 2); x++) {
            SPISW_LCD_dat8_0(0);
            SPISW_LCD_dat8_0(0);
            SPISW_LCD_dat8_0(0);
        }
    } else {
    //if (lcd_ctrl_id == ILI9340 || lcd_ctrl_id == ST7735 || lcd_ctrl_id == ST7789) {
        ILI9340_addrset((uint16_t)0, (uint16_t)0, (uint16_t)(PWX - 1), (uint16_t)(PWY - 1));
        for (y = 0; y < PWY; y++) {
            for (x = 0; x < PWX; x++) {
                SPISW_LCD_dat8_1(0);
                SPISW_LCD_dat8_1(0);
            }
        }
    }
    lcdspi_spi_end_xfer(spi_id, lcdspi);
    lcdspi->lcdspi_screen->cx = 0;
    lcdspi->lcdspi_screen->cy = 0;
    return;
}

static void lcdspi_screen_init(lcdspi_screen_t *lcdspi_screen) {
    lcdspi_screen->cx = 0;
    lcdspi_screen->cy = 0;
    lcdspi_screen->fcol = (uint16_t)0xFFFFFF;
    lcdspi_screen->bcol = (uint16_t)0x000000;
    lcdspi_screen->unit_wx = 4;
    lcdspi_screen->unit_wy = 8;
}

static void lcdspi_init(lcdspi_t *lcdspi) {
    int spi_id = lcdspi->spi_id;
    g_spi_id = spi_id;
    lcdspi_spi_init(spi_id, lcdspi);
    lcdspi_screen_init(lcdspi->lcdspi_screen);
    lcdspi_spi_start_xfer(spi_id, lcdspi);
    lcdspi->lcdspi_info->lcdspi_init();
    lcdspi_spi_end_xfer(spi_id, lcdspi);
    lcdspi_clear(lcdspi);
}

void lcdspi_deinit(lcdspi_t *lcdspi) {
    if (lcdspi->spi_id >= 0) {
        // ToDo
        //delete pspi;
    }
}

void lcdspi_bitbltex565(lcdspi_t *lcdspi, int x, int y, int width, int height, uint16_t *data) {
    int spi_id = lcdspi->spi_id;
    uint8_t PASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->PASET; // @suppress("Type cannot be resolved")
    uint8_t CASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcdspi_info->lcdspi_ctrl_info->RAMWR;
    int lcd_ctrl_id = lcdspi->lcdspi_info->lcdspi_ctrl_info->id;
    int i, j;
    uint16_t v1, v2;
    uint16_t *pdata = (uint16_t *)data;
    //SPISW_LCD_cmd(DATCTL);  // The DATCTL command selects the display mode (8-bit or 12-bit).
    lcdspi_spi_start_xfer(spi_id, lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        for (j = 0; j < height; j++) {
            SPISW_LCD_cmd8_0(PASET);
            SPISW_LCD_dat8_0((uint8_t)(y + j));
            SPISW_LCD_dat8_0((uint8_t)(y + j + 1));
            for (i = 0; i < width; i += 2) {
                SPISW_LCD_cmd8_0(CASET);
                SPISW_LCD_dat8_0((uint8_t)(x + i));
                SPISW_LCD_dat8_0((uint8_t)(x + i + 1));
                v1 = *pdata++;
                v2 = *pdata++;
                SPISW_LCD_cmd8_0(RAMWR);
                SPISW_LCD_dat8_0(R4G4(v1));
                SPISW_LCD_dat8_0(B4R4(v1, v2));
                SPISW_LCD_dat8_0(G4B4(v2));
            }
        }
    } else {
    // if (lcd_ctrl_id == ILI9340 || lcd_ctrl_id == ST7735 || lcd_ctrl_id == ST7789) {
        for (j = 0; j < height; j++) {
            ILI9340_addrset((uint16_t)x, (uint16_t)(y + j), (uint16_t)(x + width), (uint16_t)(y + j + 1));
            for (i = 0; i < width; i += 1) {
                v1 = *pdata++;
                SPISW_LCD_dat8_1((uint8_t)(v1 >> 8));
                SPISW_LCD_dat8_1((uint8_t)v1);
            }
        }

    }
    lcdspi_spi_end_xfer(spi_id, lcdspi);
}

void lcdspi_bitbltex(lcdspi_t *lcdspi, int x, int y, int width, int height, uint16_t *data) {
    int PWX = lcdspi->lcdspi_info->PWX;
    uint16_t *pdata = (uint16_t *)data;
    pdata += (y * PWX + x);
    lcdspi_bitbltex565(lcdspi, x, y, width, height, pdata);
}

void lcdspi_write_char_color(lcdspi_t *lcdspi, unsigned char c, int cx, int cy, uint16_t fgcol, uint16_t bgcol) {
    int x, y;
    int ux, uy;
    int wx, wy;
    uint16_t col0, col1;
    int spi_id = lcdspi->spi_id;
    uint8_t PASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcdspi_info->lcdspi_ctrl_info->RAMWR;
    int lcd_ctrl_id = lcdspi->lcdspi_info->lcdspi_ctrl_info->id;
    int text_sx = lcdspi->lcdspi_info->text_sx;
    int text_sy = lcdspi->lcdspi_info->text_sy;
    int disp_wx = lcdspi->lcdspi_info->disp_wx;
    int disp_wy = lcdspi->lcdspi_info->disp_wy;
    font_t *font = lcdspi->lcdspi_screen->font;
    uint8_t *data;

    if (font == (font_t *)NULL) {
        return;
    }
    if (c >= 0x80)
        c = 0;
    data = (unsigned char *)font_fontData(font, (int)(c & 0x00ff));
    ux = font_fontUnitX(font);
    uy = font_fontUnitY(font);
    wx = font_fontWidth(font, (int)(c & 0x00ff));
    wy = font_fontHeight(font, (int)(c & 0x00ff));
    lcdspi_spi_start_xfer(spi_id, lcdspi);
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        for (y = 0; y < wy; y++) {
            SPISW_LCD_cmd8_0(CASET);
            SPISW_LCD_dat8_0((uint8_t)(cx * ux + text_sx));
            SPISW_LCD_dat8_0((uint8_t)(disp_wx - 1));
            SPISW_LCD_cmd8_0(PASET);		//y set
            SPISW_LCD_dat8_0((uint8_t)(cy * uy + y + text_sy));
            SPISW_LCD_dat8_0((uint8_t)(disp_wy - 1));
            SPISW_LCD_cmd8_0(RAMWR);
            for (x = 0; x < (wx / 2); x++) {
                if (data[y] & (0x80 >> (x * 2))) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                if (data[y] & (0x40 >> (x * 2))) {
                    col1 = fgcol;
                } else {
                    col1 = bgcol;
                }
                SPISW_LCD_dat8_0((0xff & (uint8_t)(col0 >> 4)));
                SPISW_LCD_dat8_0((0xf0 & (uint8_t)(col0 << 4)) | (0x0f & ((uint8_t)(col1 >> 8))));
                SPISW_LCD_dat8_0((uint8_t)(0xff & col1));
            }
        }
    } else {
    //if (lcd_ctrl_id == ILI9340 || lcd_ctrl_id == ST7735 || lcd_ctrl_id == ST7789) {
        for (y = 0; y < wy; y++) {
            ILI9340_addrset((uint16_t)(cx * ux + text_sx),
                (uint16_t)(cy * uy + y + text_sy),
                (uint16_t)(disp_wx - 1),
                (uint16_t)(disp_wy - 1));
            for (x = 0; x < wx; x++) {
                if (data[y] & (0x80 >> x)) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                SPISW_LCD_dat8_1((uint8_t)(col0 >> 8));
                SPISW_LCD_dat8_1((uint8_t)col0);
            }
        }
    }
    lcdspi_spi_end_xfer(spi_id, lcdspi);
}

void lcdspi_write_unicode_color(lcdspi_t *lcdspi, unsigned short u, int cx, int cy, uint16_t fgcol, uint16_t bgcol) {
    int x, y;
    int ux, uy;
    int wx, wy;
    int off;
    uint16_t col0, col1;
    int spi_id = lcdspi->spi_id;
    uint8_t PASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->PASET;
    uint8_t CASET = lcdspi->lcdspi_info->lcdspi_ctrl_info->CASET;
    uint8_t RAMWR = lcdspi->lcdspi_info->lcdspi_ctrl_info->RAMWR;
    int lcd_ctrl_id = lcdspi->lcdspi_info->lcdspi_ctrl_info->id;
    int text_sx = lcdspi->lcdspi_info->text_sx;
    int text_sy = lcdspi->lcdspi_info->text_sy;
    int disp_wx = lcdspi->lcdspi_info->disp_wx;
    int disp_wy = lcdspi->lcdspi_info->disp_wy;
    font_t *font = lcdspi->lcdspi_screen->font;
    uint8_t *data;

    if (font == (font_t *)NULL) {
        return;
    }
    data = (unsigned char *)font_fontData(font, (int)u);
    ux = font_fontUnitX(font);
    uy = font_fontUnitY(font);
    wx = font_fontWidth(font, (int)u);
    wy = font_fontHeight(font, (int)u);
    lcdspi_spi_start_xfer(spi_id, lcdspi);
    off = 0;
    if (lcd_ctrl_id == PCF8833 || lcd_ctrl_id == S1D15G10) {
        for (y = 0; y < wy; y++) {
            SPISW_LCD_cmd8_0(CASET);
            SPISW_LCD_dat8_0(cx * ux + text_sx);
            SPISW_LCD_dat8_0(disp_wx - 1);
            SPISW_LCD_cmd8_0(PASET);		//y set
            SPISW_LCD_dat8_0(cy * uy + y + text_sy);
            SPISW_LCD_dat8_0(disp_wy - 1);
            SPISW_LCD_cmd8_0(RAMWR);
            for (x = 0; x < wx; x += 2) {
                if (x == 8) {
                    off++;
                }
                if (data[off] & (0x80 >> (x & 0x7))) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                if (data[off] & (0x40 >> (x & 0x7))) {
                    col1 = fgcol;
                } else {
                    col1 = bgcol;
                }
                SPISW_LCD_dat8_0((0xff & (uint8_t)(col0 >> 4)));
                SPISW_LCD_dat8_0((0xf0 & (uint8_t)(col0 << 4)) | (0x0f & ((uint8_t)(col1 >> 8))));
                SPISW_LCD_dat8_0((uint8_t)(0xff & col1));
            }
            off++;
        }
    } else {
    //if (lcd_ctrl_id == ILI9340 || lcd_ctrl_id == ST7735 || lcd_ctrl_id == ST7735) {
        for (y = 0; y < wy; y++) {
            ILI9340_addrset((uint16_t)(cx * ux + text_sx),
                (uint16_t)(cy * uy + y + text_sy),
                (uint16_t)(disp_wx - 1),
                (uint16_t)(disp_wy - 1));
            for (x = 0; x < wx; x++) {
                if (x == 8) {
                    off++;
                }
                if (data[off] & (0x80 >> (x & 0x7))) {
                    col0 = fgcol;
                } else {
                    col0 = bgcol;
                }
                SPISW_LCD_dat8_1((uint8_t)(col0 >> 8));
                SPISW_LCD_dat8_1((uint8_t)col0);
            }
            off++;
        }
    }
    lcdspi_spi_end_xfer(spi_id, lcdspi);
}

void lcdspi_write_char(lcdspi_t *lcdspi, unsigned char c, int row, int col) {
    lcdspi_write_char_color(lcdspi, c, row, col, lcdspi->lcdspi_screen->fcol, lcdspi->lcdspi_screen->bcol);
}

void lcdspi_write_unicode(lcdspi_t *lcdspi, unsigned short u, int row, int col) {
    lcdspi_write_unicode_color(lcdspi, u, row, col, lcdspi->lcdspi_screen->fcol, lcdspi->lcdspi_screen->bcol);
}

void lcdspi_write_formatted_char(lcdspi_t *lcdspi, unsigned char ch) {
    lcdspi_screen_t *screen = lcdspi->lcdspi_screen;
    int cx = screen->cx;
    int cy = screen->cy;
    int unit_x = font_fontUnitX(screen->font);
    int unit_y = font_fontUnitY(screen->font);
    int disp_wx = lcdspi->lcdspi_info->disp_wx;
    int disp_wy = lcdspi->lcdspi_info->disp_wy;
    if (ch == 0xc) {
        lcdspi_clear(lcdspi);
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
        lcdspi_write_char(lcdspi, ch, cx, cy);
        cx++;
        if (cx == disp_wx / unit_x) {
            cx = 0;
            cy++;
            if (cy == disp_wy / unit_y) {
                cy = 0;
            }
        }
    }
    screen->cx = cx;
    screen->cy = cy;
}

void lcdspi_write_formatted_unicode(lcdspi_t *lcdspi, unsigned short u) {
    lcdspi_screen_t *screen = lcdspi->lcdspi_screen;
    int cx = screen->cx;
    int cy = screen->cy;
    int unit_x = font_fontUnitX(screen->font);
    int unit_y = font_fontUnitY(screen->font);
    int disp_wx = lcdspi->lcdspi_info->disp_wx;
    int disp_wy = lcdspi->lcdspi_info->disp_wy;
      if ((char)u == 0xc) {
        lcdspi_clear(lcdspi);
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
        lcdspi_write_unicode(lcdspi, u, cx, cy);
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
    screen->cx = cx;
    screen->cy = cy;
}

void lcdspi_set_font(lcdspi_t *lcdspi, font_t *font) {
   lcdspi->lcdspi_screen->font = font;
}

font_t *lcdspi_get_font(lcdspi_t *lcdspi) {
    return lcdspi->lcdspi_screen->font;
}

/******************************************************************************/
/* MicroPython bindings                                                       */

void lcdspi_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "LCDSPI: %s(%d, %d) FONT: %s",
        self->lcdspi->lcdspi_info->name,
        self->lcdspi->lcdspi_info->PWX,
        self->lcdspi->lcdspi_info->PWY,
        self->lcdspi_screen.font->fontName);
}

unsigned short cnvUtf8ToUnicode(unsigned char *str, int *size)
{
    unsigned int u = 0;
    unsigned char c = *str++;
    int len = 0;
    if ((c & 0x80) == 0) {
        u = c & 0x7F;
        len = 0;
    } else if ((c & 0xE0) == 0xC0) {
        u = c & 0x1F;
        len = 1;
    } else if ((c & 0xF0) == 0xE0) {
        u = c & 0x0F;
        len = 2;
    } else if ((c & 0xF8) == 0xF0) {
        u = c & 0x07;
        len = 3;
    } else if ((c & 0xFC) == 0xF8) {
        u = c & 0x03;
        len = 4;
    } else if ((c & 0xFE) == 0xFC) {
        u = c & 0x01;
        len = 5;
    }
    *size = len + 1;
    while (len-- > 0 && ((c = *str) & 0xC0) == 0x80) {
        u = (u << 6) | (unsigned int)(c & 0x3F);
        str++;
    }
    return (unsigned short)u;
}

static int BYTEARRAY4_TO_INT(char *a) {
    return (int)*(uint32_t *)a;
}

static int BYTEARRAY2_TO_INT(char *a) {
    return (int)*(uint16_t *)a;
}

int lcdspi_disp_bmp_sd(lcdspi_t *lcdspi, int x, int y, const char *filename) {
    #define BMP_HEADER_SIZE 0x8a
    char BmpHeader[BMP_HEADER_SIZE];
    FIL fp;
    FRESULT res;
    fs_user_mount_t *vfs_fat;
    const char *p_out;
    int ofs, wx, wy, depth, lineBytes, bufSize;
    int bitmapDy = 1;

    mp_vfs_mount_t *vfs = mp_vfs_lookup_path(filename, &p_out);
    if (vfs != MP_VFS_NONE && vfs != MP_VFS_ROOT) {
        vfs_fat = MP_OBJ_TO_PTR(vfs->obj);
    } else {
#if defined(DEBUG_LCDSPI)
        debug_printf("Cannot find user mount for %s\n", filename);
#endif
        return -1;
    }
    res = f_open(&vfs_fat->fatfs, &fp, filename, FA_READ);
    if (res != FR_OK) {
#if defined(DEBUG_LCDSPI)
        debug_printf("File can't be opened", filename);
#endif
        return -1;
    }
    uint32_t readed;
    f_read(&fp, (void *)BmpHeader, (UINT)BMP_HEADER_SIZE, (UINT *)&readed);
    ofs = BYTEARRAY4_TO_INT((char *)&BmpHeader[0x0a]);
    wx = BYTEARRAY4_TO_INT((char *)&BmpHeader[0x12]);
    wy = BYTEARRAY4_TO_INT((char *)&BmpHeader[0x16]);
    depth = BYTEARRAY2_TO_INT((char *)&BmpHeader[0x1c]);
    lineBytes = wx * depth / 8;
    bufSize = lineBytes * bitmapDy;
#if defined(DEBUG_LCDSPI)
    debug_printf("wx=", wx); debug_printf("wy=", wy); debug_printf("depth=", depth); debug_printf("bufSize=", bufSize);
#endif
    unsigned char *bitmapOneLine = (unsigned char *)malloc(bufSize);
    if (!bitmapOneLine) {
#if defined(DEBUG_LCDSPI)
        debug_printf("Memory allocation failed.");
#endif
        f_close(&fp);
        return -1;
    }
    f_lseek (&fp, (FSIZE_t)ofs);
    if (depth == 16) {
        for (int ty = wy - 1 - bitmapDy; ty >= 0; ty -= bitmapDy) {
            f_read(&fp, (void *)bitmapOneLine, (UINT)bufSize, (UINT *)&readed);
            lcdspi_bitbltex565(lcdspi, x, y + ty, wx, bitmapDy, (uint16_t *)bitmapOneLine);
        }
    } else if (depth == 24) {
        for (int ty = wy - 1 - bitmapDy; ty >= 0; ty -= bitmapDy) {
            f_read(&fp, (void *)bitmapOneLine, (UINT)bufSize, (UINT *)&readed);
            for (int i = 0; i < wx; i++) {
                uint16_t b = (uint16_t)bitmapOneLine[i * 3];
                uint16_t g = (uint16_t)bitmapOneLine[i * 3 + 1];
                uint16_t r = (uint16_t)bitmapOneLine[i * 3 + 2];
                uint16_t v = (b >> 3) + ((g >> 2) << 5) + ((r >> 3) << 11);
                bitmapOneLine[i * 2] = (uint8_t)(v);
                bitmapOneLine[i * 2 + 1] = (uint8_t)(v >> 8);
            }
            lcdspi_bitbltex565(lcdspi, x, y + ty, wx, bitmapDy, (uint16_t *)bitmapOneLine);
        }
    }
    if (bitmapOneLine) {
        free(bitmapOneLine);
    }
    return 1;
}

int lcdspi_disp_jpeg_sd(lcdspi_t *lcdspi, int x, int y, const char *filename) {
    jpeg_t jpeg;
    uint8_t *img;
    int cx, cy;
    int sx, sy;
    int decoded_width;
    //int decoded_height;
    int dDiv = 2;
    int split_disp = 1;
    int split = 0;
    int alloc_size = 0;
    int MCUWidth;
    int MCUHeight;
    int width;
    int height;
    int _disp_wx = lcdspi->lcdspi_info->disp_wx;
    int _disp_wy = lcdspi->lcdspi_info->disp_wy;
    unsigned short *dispBuf = (unsigned short *)NULL;
    int tx, ty;

    jpeg_init(&jpeg);
    jpeg_decode(&jpeg, (char *)filename, split);
    if (jpeg.err != 0) {
#if defined(DEBUG_LCDSPI)
        debug_printf("jpeg decode error");
#endif
        return -1;
    }
    decoded_width = jpeg.decoded_width;
    //decoded_height = jpeg.decoded_height;
    MCUWidth = jpeg.image_info.m_MCUWidth;
    MCUHeight = jpeg.image_info.m_MCUHeight;
    width = jpeg.image_info.m_width;
    height = jpeg.image_info.m_height;
    if (split_disp) {
        alloc_size = MCUWidth * MCUHeight * sizeof(unsigned short);
    } else {
        // alloc buf for jpeg size
        alloc_size = width * height * sizeof(unsigned short);
    }
    dispBuf = (unsigned short *)malloc(alloc_size);
    if (!dispBuf) {
#if defined(DEBUG_LCDSPI)
        debug_printf("dispBuf allocation failed.");
#endif
        return -1;
    } else {
#if defined(DEBUG_LCDSPI)
        debug_printf("dispBuf allocated", alloc_size);
#endif
    }
    while (jpeg_read(&jpeg)) {
        img = jpeg.pImage;
        if (!img) {
            break;
        }
        sx = jpeg.MCUx * MCUWidth;
        //sy = jpeg.MCUy * jpeg.MCUHeight() % (_disp_wx / dDiv);
        sy = jpeg.MCUy * MCUHeight;
        memset(dispBuf, 0, alloc_size);
        for (ty = 0; ty < MCUHeight; ty++) {
            for (tx = 0; tx < MCUWidth; tx++) {
                cx = sx + tx;
                cy = sy + ty;
                if ((cx < width) && (cy < height)) {
                    if (jpeg.comps == 1) {
                        if ((cx < _disp_wx) && (cy < _disp_wy / dDiv)) {
                            if (split_disp) {
#if defined(DEBUG_LCDSPI)
                                debug_printf("ofs: %d\r\n", ty * MCUWidth + tx);
#endif
                                dispBuf[ty * MCUWidth + tx] = (((img[0] >> 3) << 11)) | (((img[0] >> 2) << 5)) | ((img[0] >> 3));
                            } else {
                                dispBuf[cy * decoded_width + cx] = (((img[0] >> 3) << 11)) | (((img[0] >> 2) << 5)) | ((img[0] >> 3));
                            }
                        }
                    } else {
                        //if ((cx < _disp_wx) && (cy < _disp_wy/dDiv)) {
                        if (split_disp) {
                            //debug_printf("ofs", ty * MCUWidth  + tx);
                            dispBuf[ty * MCUWidth + tx] = (((img[0] >> 3) << 11)) | (((img[1] >> 2) << 5)) | ((img[2] >> 3));
                        } else {
                            dispBuf[cy * decoded_width + cx] = (((img[0] >> 3) << 11)) | (((img[1] >> 2) << 5)) | ((img[2] >> 3));
                        }
                        //}
                    }
                }
                img += jpeg.comps;
            }
        }
        if (split_disp) {
            //debug_printf("sx", sx);
            //debug_printf("sy", sy);
            int disp_height;
            if ((jpeg.MCUy + 1) * MCUHeight > height) {
                disp_height = height - sy;
            } else {
                disp_height = MCUHeight;
            }
            lcdspi_bitbltex565(lcdspi, x + sx, y + sy, MCUWidth, disp_height, (uint16_t *)dispBuf);
        }
    }
#if defined(DEBUG_LCDSPI)
    debug_printf("err=", jpeg.err);
#endif
    if (jpeg.err == 0 || jpeg.err == PJPG_NO_MORE_BLOCKS) {
        if (!split_disp) {
            lcdspi_bitbltex565(lcdspi, x, y, width, height, (uint16_t *)dispBuf);
        }
    }
    if (dispBuf) {
#if defined(DEBUG_LCDSPI)
        debug_printf("dispBuf deallocated");
#endif
        free(dispBuf);
    }
    return 1;
}

/// \classmethod \constructor(id)
/// Create an LCDSPI object
///
STATIC mp_obj_t lcdspi_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_lcd_id, ARG_font_id, ARG_spi_id, ARG_baud, ARG_cs, ARG_clk, ARG_dout, ARG_reset, ARG_rs, ARG_din, ARG_polarity, ARG_phase };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_lcd_id,   MP_ARG_REQUIRED | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_font_id,  MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 0} },
        { MP_QSTR_spi_id,   MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = -1} },
        { MP_QSTR_baud,     MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 4000000} },
        { MP_QSTR_cs,       MP_ARG_KW_ONLY  | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_clk,      MP_ARG_KW_ONLY  | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_dout,     MP_ARG_KW_ONLY  | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_reset,    MP_ARG_KW_ONLY  | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rs,       MP_ARG_KW_ONLY  | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_din,      MP_ARG_KW_ONLY  | MP_ARG_OBJ,   {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 1} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 0} }
    };
    // parse args
    mp_arg_val_t vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, vals);

    pyb_lcdspi_obj_t *self = m_new_obj(pyb_lcdspi_obj_t);
    self->base.type = type;
    /* set lcdspi  */
    self->lcdspi_id = vals[ARG_lcd_id].u_int;
    self->lcdspi = &lcdspi_all[self->lcdspi_id];
    self->lcdspi->lcdspi_screen = &self->lcdspi_screen;
    self->lcdspi->lcdspi_pins = &self->lcdspi_pins;
    /* set font */
    int font_id = vals[ARG_font_id].u_int;
    if (!find_font_id(font_id)) {
        mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("FONT(%d) doesn't exist"), font_id);
    }
    self->lcdspi_screen.font = get_font_by_id(font_id);
    /* set spi hw channel should be minus 1 */
    self->lcdspi->spi_id = vals[ARG_spi_id].u_int - 1;
    if (self->lcdspi->spi_id > 2) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("SPI_ID <= 2"));
    }
    /* set spi default pins */
    self->lcdspi_pins = lcdspi_pins_def;
    /* set spi pins from input */
    /* baud */
    self->lcdspi->baud = vals[ARG_baud].u_int;
    self->lcdspi->polarity = vals[ARG_polarity].u_int;
    self->lcdspi->phase = vals[ARG_phase].u_int;
    /* cs */
    if (vals[ARG_cs].u_obj == MP_OBJ_NULL) {
        _csPin = self->lcdspi_pins.csPin->pin;
    } else if (!mp_obj_is_type(vals[ARG_cs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->lcdspi_pins.csPin = MP_OBJ_TO_PTR(vals[ARG_cs].u_obj);
        _csPin = self->lcdspi_pins.csPin->pin;
    }
    /* clk */
    if (vals[ARG_clk].u_obj == MP_OBJ_NULL) {
        _clkPin = self->lcdspi_pins.clkPin->pin;
    } else if (!mp_obj_is_type(vals[ARG_clk].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->lcdspi_pins.clkPin = MP_OBJ_TO_PTR(vals[ARG_clk].u_obj);
        _clkPin = self->lcdspi_pins.clkPin->pin;
    }
    /* dout */
    if (vals[ARG_dout].u_obj == MP_OBJ_NULL) {
        _doutPin = self->lcdspi_pins.doutPin->pin;
    } else if (!mp_obj_is_type(vals[ARG_dout].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->lcdspi_pins.doutPin = MP_OBJ_TO_PTR(vals[ARG_dout].u_obj);
        _doutPin = self->lcdspi_pins.doutPin->pin;
    }
    /* reset */
    if (vals[ARG_reset].u_obj == MP_OBJ_NULL) {
        _resetPin = self->lcdspi_pins.resetPin->pin;
    } else if (!mp_obj_is_type(vals[ARG_reset].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->lcdspi_pins.resetPin = MP_OBJ_TO_PTR(vals[ARG_reset].u_obj);
        _resetPin = self->lcdspi_pins.resetPin->pin;
    }
    /* rs */
    if (vals[ARG_rs].u_obj == MP_OBJ_NULL) {
        _rsPin = self->lcdspi_pins.rsPin->pin;
    } else if (!mp_obj_is_type(vals[ARG_rs].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->lcdspi_pins.rsPin = MP_OBJ_TO_PTR(vals[ARG_rs].u_obj);
        _rsPin = self->lcdspi_pins.rsPin->pin;
    }
    /* din */
    if (vals[ARG_din].u_obj == MP_OBJ_NULL) {
        _dinPin = self->lcdspi_pins.dinPin->pin;
    } else if (!mp_obj_is_type(vals[ARG_din].u_obj, &pin_type)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("This is not Pin obj"));
    } else {
        self->lcdspi_pins.dinPin = MP_OBJ_TO_PTR(vals[ARG_din].u_obj);
        _dinPin = self->lcdspi_pins.dinPin->pin;
    }
    lcdspi_init(self->lcdspi);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t pyb_lcdspi_clear(mp_obj_t self_in)  {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    lcdspi_clear(self->lcdspi);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_lcdspi_clear_obj, pyb_lcdspi_clear);

STATIC mp_obj_t pyb_lcdspi_font_id(mp_obj_t self_in, mp_obj_t idx) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_id = mp_obj_get_int(idx);
    font_t *font = get_font_by_id(font_id);
    if (font) {
        lcdspi_set_font(self->lcdspi, font);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_lcdspi_font_id_obj, pyb_lcdspi_font_id);

STATIC mp_obj_t pyb_lcdspi_putxy(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *s = (char *)mp_obj_str_get_str(args[3]);
    lcdspi_write_char(self->lcdspi, *s, x, y);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_putxy_obj, 4, 4, pyb_lcdspi_putxy);

STATIC mp_obj_t pyb_lcdspi_putc(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    char *s = (char *)mp_obj_str_get_str(args[1]);
    lcdspi_write_formatted_char(self->lcdspi, *s);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_putc_obj, 2, 2, pyb_lcdspi_putc);

STATIC mp_obj_t pyb_lcdspi_puts(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    char *s = (char *)mp_obj_str_get_str(args[1]);
    while (*s) {
        lcdspi_write_formatted_char(self->lcdspi, *s++);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_puts_obj, 2, 2, pyb_lcdspi_puts);

STATIC mp_obj_t pyb_lcdspi_pututf8(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    uint16_t u;
    int len;
    char *s = (char *)mp_obj_str_get_str(args[1]);
    while (*s) {
        u = cnvUtf8ToUnicode((unsigned char *)s, &len);
        lcdspi_write_formatted_unicode(self->lcdspi, u);
        s += len;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_pututf8_obj, 2, 2, pyb_lcdspi_pututf8);

STATIC mp_obj_t pyb_lcdspi_bitblt(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int len;
    mp_obj_t *o;
    char *data;
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    int wx = mp_obj_get_int(args[3]);
    int wy = mp_obj_get_int(args[4]);
    mp_obj_get_array(args[5], (size_t *)&len, &o);
    // ToDo: get byte array pointer
    data = (char *)o;
    lcdspi_bitbltex(self->lcdspi, x, y, wx, wy, (uint16_t *)data);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_bitblt_obj, 6, 6, pyb_lcdspi_bitblt);

STATIC mp_obj_t pyb_lcdspi_disp_bmp_sd(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *fn = (char *)mp_obj_str_get_str(args[3]);
    lcdspi_disp_bmp_sd(self->lcdspi, x, y, fn);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_disp_bmp_sd_obj, 4, 4, pyb_lcdspi_disp_bmp_sd);

STATIC mp_obj_t pyb_lcdspi_disp_jpeg_sd(size_t n_args, const mp_obj_t *args) {
    pyb_lcdspi_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    char *fn = (char *)mp_obj_str_get_str(args[3]);
    lcdspi_disp_jpeg_sd(self->lcdspi, x, y, fn);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_lcdspi_disp_jpeg_sd_obj, 4, 4, pyb_lcdspi_disp_jpeg_sd);

STATIC const mp_rom_map_elem_t lcdspi_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&pyb_lcdspi_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_font_id), MP_ROM_PTR(&pyb_lcdspi_font_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_putxy), MP_ROM_PTR(&pyb_lcdspi_putxy_obj) },
    { MP_ROM_QSTR(MP_QSTR_putc), MP_ROM_PTR(&pyb_lcdspi_putc_obj) },
    { MP_ROM_QSTR(MP_QSTR_puts), MP_ROM_PTR(&pyb_lcdspi_puts_obj) },
    { MP_ROM_QSTR(MP_QSTR_pututf8), MP_ROM_PTR(&pyb_lcdspi_pututf8_obj) },
    { MP_ROM_QSTR(MP_QSTR_bitblt), MP_ROM_PTR(&pyb_lcdspi_bitblt_obj) },
    { MP_ROM_QSTR(MP_QSTR_disp_bmp_sd), MP_ROM_PTR(&pyb_lcdspi_disp_bmp_sd_obj) },
    { MP_ROM_QSTR(MP_QSTR_disp_jpeg_sd), MP_ROM_PTR(&pyb_lcdspi_disp_jpeg_sd_obj) },
    { MP_ROM_QSTR(MP_QSTR_C_PCF8833), MP_ROM_INT(PCF8833) },
    { MP_ROM_QSTR(MP_QSTR_C_S1D15G10), MP_ROM_INT(S1D15G10) },
    { MP_ROM_QSTR(MP_QSTR_C_ILI9341), MP_ROM_INT(ILI9341) },
    { MP_ROM_QSTR(MP_QSTR_C_ILI9340), MP_ROM_INT(ILI9340) },
    { MP_ROM_QSTR(MP_QSTR_C_ST7735), MP_ROM_INT(ST7735) },
    { MP_ROM_QSTR(MP_QSTR_C_ST7789), MP_ROM_INT(ST7789) },
    { MP_ROM_QSTR(MP_QSTR_M_NOKIA6100_0), MP_ROM_INT(NOKIA6100_0) },
    { MP_ROM_QSTR(MP_QSTR_M_NOKIA6100_1), MP_ROM_INT(NOKIA6100_1) },
    { MP_ROM_QSTR(MP_QSTR_M_T180), MP_ROM_INT(T180) },
    { MP_ROM_QSTR(MP_QSTR_M_M022C9340SPI), MP_ROM_INT(M022C9340SPI) },
    { MP_ROM_QSTR(MP_QSTR_M_RASPI13LCDSPI), MP_ROM_INT(RASPI13LCDSPI) },
    { MP_ROM_QSTR(MP_QSTR_M_RASPI28LCDSPI), MP_ROM_INT(RASPI28LCDSPI) },
};

STATIC MP_DEFINE_CONST_DICT(lcdspi_locals_dict, lcdspi_locals_dict_table);

const mp_obj_type_t pyb_lcdspi_type = {
    { &mp_type_type },
    .name = MP_QSTR_LCDSPI,
    .print = lcdspi_obj_print,
    .make_new = lcdspi_obj_make_new,
    .locals_dict = (mp_obj_dict_t*)&lcdspi_locals_dict,
};

#endif
