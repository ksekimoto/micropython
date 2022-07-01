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

#include <stdbool.h>
#include "mbed.h"
#include "rtos.h"
#include "dcache-control.h"
#include "PinNames.h"
#include "DisplayBace.h"
#ifdef MBED_CONF_APP_LCD
#undef MBED_CONF_APP_LCD
#endif
#define MBED_CONF_APP_LCD   (1)
#include "EasyAttach_CameraAndLCD.h"
#include "rz_buf.h"
#include "display.h"
#include "mbed_lcd.h"

DisplayBase g_mbed_display;
bool m_lcd_initialized = false;

#if defined(TARGET_GR_MANGO)
  #define LCD_H_SYNC_PORT   DisplayBase::LCD_TCON_PIN_0
  #define LCD_V_SYNC_PORT   DisplayBase::LCD_TCON_PIN_1
  #define LCD_DE_PORT       DisplayBase::LCD_TCON_PIN_2
#elif defined(TARGET_RZ_A2XX)
  #define LCD_H_SYNC_PORT   DisplayBase::LCD_TCON_PIN_3
  #define LCD_V_SYNC_PORT   DisplayBase::LCD_TCON_PIN_0
  #define LCD_DE_PORT       DisplayBase::LCD_TCON_PIN_4
#else
  #define LCD_H_SYNC_PORT   DisplayBase::LCD_TCON_PIN_NON
  #define LCD_V_SYNC_PORT   DisplayBase::LCD_TCON_PIN_NON
  #define LCD_DE_PORT       DisplayBase::LCD_TCON_PIN_NON
#endif

static const DisplayBase::lcd_config_t LcdCfgTbl_Display_shield_800x600 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SL_SVGA_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SL_SVGA_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (SL_SVGA_LCD_PIXEL_WIDTH + SL_SVGA_LCD_H_FRONT_PORCH + SL_SVGA_LCD_H_BACK_PORCH + SL_SVGA_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SL_SVGA_LCD_PIXEL_HEIGHT + SL_SVGA_LCD_V_FRONT_PORCH + SL_SVGA_LCD_V_BACK_PORCH + SL_SVGA_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SL_SVGA_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SL_SVGA_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , SL_SVGA_LCD_H_BACK_PORCH                                                              /* h_back_porch         */
    , SL_SVGA_LCD_V_BACK_PORCH                                                              /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_1                                                   /* h_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* h_sync_port_polarity */
    , SL_SVGA_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_2                                                   /* v_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* v_sync_port_polarity */
    , SL_SVGA_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_0                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_Display_shield_1024x768 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SL_XGA_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SL_SVGA_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (SL_SVGA_LCD_PIXEL_WIDTH + SL_SVGA_LCD_H_FRONT_PORCH + SL_SVGA_LCD_H_BACK_PORCH + SL_SVGA_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SL_SVGA_LCD_PIXEL_HEIGHT + SL_SVGA_LCD_V_FRONT_PORCH + SL_SVGA_LCD_V_BACK_PORCH + SL_SVGA_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SL_SVGA_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SL_SVGA_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , SL_SVGA_LCD_H_BACK_PORCH                                                              /* h_back_porch         */
    , SL_SVGA_LCD_V_BACK_PORCH                                                              /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_1                                                   /* h_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* h_sync_port_polarity */
    , SL_SVGA_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_2                                                   /* v_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* v_sync_port_polarity */
    , SL_SVGA_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_0                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_Display_shield_1280x720 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SL_HD_720P_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SL_HD_720P_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (SL_HD_720P_LCD_PIXEL_WIDTH + SL_HD_720P_LCD_H_FRONT_PORCH + SL_HD_720P_LCD_H_BACK_PORCH + SL_HD_720P_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SL_HD_720P_LCD_PIXEL_HEIGHT + SL_HD_720P_LCD_V_FRONT_PORCH + SL_HD_720P_LCD_V_BACK_PORCH + SL_HD_720P_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SL_HD_720P_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SL_HD_720P_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , SL_HD_720P_LCD_H_BACK_PORCH                                                              /* h_back_porch         */
    , SL_HD_720P_LCD_V_BACK_PORCH                                                              /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_1                                                   /* h_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* h_sync_port_polarity */
    , SL_HD_720P_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_2                                                   /* v_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* v_sync_port_polarity */
    , SL_HD_720P_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_0                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

// LcdCfg_40pin
static const DisplayBase::lcd_config_t LcdCfgTbl_40pin_4_3inch_480x272 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , TFT_43_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , TFT_43_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (TFT_43_LCD_PIXEL_WIDTH + TFT_43_LCD_H_FRONT_PORCH + TFT_43_LCD_H_BACK_PORCH)                      /* h_toatal_period      */
    , (TFT_43_LCD_PIXEL_HEIGHT + TFT_43_LCD_V_FRONT_PORCH + TFT_43_LCD_V_BACK_PORCH)                     /* v_toatal_period      */
    , TFT_43_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , TFT_43_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , TFT_43_LCD_H_BACK_PORCH                                                              /* h_back_porch         */
    , TFT_43_LCD_V_BACK_PORCH                                                              /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_0                                                   /* h_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* h_sync_port_polarity */
    , 0                                                                             /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_1                                                   /* v_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* v_sync_port_polarity */
    , 0                                                                             /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_2                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_40pin_5inch_800x480 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , TFT_5_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , TFT_5_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (TFT_5_LCD_PIXEL_WIDTH + TFT_5_LCD_H_FRONT_PORCH + TFT_5_LCD_H_BACK_PORCH)                      /* h_toatal_period      */
    , (TFT_5_LCD_PIXEL_HEIGHT + TFT_5_LCD_V_FRONT_PORCH + TFT_5_LCD_V_BACK_PORCH)                     /* v_toatal_period      */
    , TFT_5_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , TFT_5_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , TFT_5_LCD_H_BACK_PORCH                                                              /* h_back_porch         */
    , TFT_5_LCD_V_BACK_PORCH                                                              /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_0                                                   /* h_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* h_sync_port_polarity */
    , 0                                                                             /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_1                                                   /* v_sync_port          */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* v_sync_port_polarity */
    , 0                                                                             /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_2                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_RSK_TFT_800x480 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , RSK_TFT_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , RSK_TFT_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_RISING                                                      /* lcd_edge             */
    , (RSK_TFT_LCD_PIXEL_WIDTH + RSK_TFT_LCD_H_FRONT_PORCH + RSK_TFT_LCD_H_BACK_PORCH + RSK_TFT_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (RSK_TFT_LCD_PIXEL_HEIGHT + RSK_TFT_LCD_V_FRONT_PORCH + RSK_TFT_LCD_V_BACK_PORCH + RSK_TFT_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , RSK_TFT_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , RSK_TFT_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (RSK_TFT_LCD_H_BACK_PORCH + RSK_TFT_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (RSK_TFT_LCD_V_BACK_PORCH + RSK_TFT_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , DisplayBase::LCD_TCON_PIN_3                                                   /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , RSK_TFT_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , DisplayBase::LCD_TCON_PIN_0                                                   /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , RSK_TFT_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , DisplayBase::LCD_TCON_PIN_4                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_rgb_to_hdmi_800x480 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SD_7INCH_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SD_7INCH_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (SD_7INCH_LCD_PIXEL_WIDTH + SD_7INCH_LCD_H_FRONT_PORCH + SD_7INCH_LCD_H_BACK_PORCH + SD_7INCH_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SD_7INCH_LCD_PIXEL_HEIGHT + SD_7INCH_LCD_V_FRONT_PORCH + SD_7INCH_LCD_V_BACK_PORCH + SD_7INCH_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SD_7INCH_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SD_7INCH_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (SD_7INCH_LCD_H_BACK_PORCH + SD_7INCH_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (SD_7INCH_LCD_V_BACK_PORCH + SD_7INCH_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , SD_7INCH_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , SD_7INCH_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_rgb_to_hdmi_800x600 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SVGA_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SVGA_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (SVGA_LCD_PIXEL_WIDTH + SVGA_LCD_H_FRONT_PORCH + SVGA_LCD_H_BACK_PORCH + SVGA_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SVGA_LCD_PIXEL_HEIGHT + SVGA_LCD_V_FRONT_PORCH + SVGA_LCD_V_BACK_PORCH + SVGA_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SVGA_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SVGA_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (SVGA_LCD_H_BACK_PORCH + SVGA_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (SVGA_LCD_V_BACK_PORCH + SVGA_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , SVGA_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , SVGA_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_rgb_to_hdmi_1024x768 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , XGA_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , XGA_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (XGA_LCD_PIXEL_WIDTH + XGA_LCD_H_FRONT_PORCH + XGA_LCD_H_BACK_PORCH + XGA_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (XGA_LCD_PIXEL_HEIGHT + XGA_LCD_V_FRONT_PORCH + XGA_LCD_V_BACK_PORCH + XGA_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , XGA_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , XGA_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (XGA_LCD_H_BACK_PORCH + XGA_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (XGA_LCD_V_BACK_PORCH + XGA_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , XGA_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , XGA_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_rgb_to_hdmi_1280x720 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , HD_720P_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , HD_720P_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (HD_720P_LCD_PIXEL_WIDTH + HD_720P_LCD_H_FRONT_PORCH + HD_720P_LCD_H_BACK_PORCH + HD_720P_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (HD_720P_LCD_PIXEL_HEIGHT + HD_720P_LCD_V_FRONT_PORCH + HD_720P_LCD_V_BACK_PORCH + HD_720P_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , HD_720P_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , HD_720P_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (HD_720P_LCD_H_BACK_PORCH + HD_720P_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (HD_720P_LCD_V_BACK_PORCH + HD_720P_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , HD_720P_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , HD_720P_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_lvds_to_hdmi_800x480 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SD_7INCH_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SD_7INCH_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (SD_7INCH_LCD_PIXEL_WIDTH + SD_7INCH_LCD_H_FRONT_PORCH + SD_7INCH_LCD_H_BACK_PORCH + SD_7INCH_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SD_7INCH_LCD_PIXEL_HEIGHT + SD_7INCH_LCD_V_FRONT_PORCH + SD_7INCH_LCD_V_BACK_PORCH + SD_7INCH_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SD_7INCH_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SD_7INCH_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (SD_7INCH_LCD_H_BACK_PORCH + SD_7INCH_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (SD_7INCH_LCD_V_BACK_PORCH + SD_7INCH_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , SD_7INCH_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , SD_7INCH_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_lvds_to_hdmi_800x600 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , SVGA_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , SVGA_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (SVGA_LCD_PIXEL_WIDTH + SVGA_LCD_H_FRONT_PORCH + SVGA_LCD_H_BACK_PORCH + SVGA_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (SVGA_LCD_PIXEL_HEIGHT + SVGA_LCD_V_FRONT_PORCH + SVGA_LCD_V_BACK_PORCH + SVGA_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , SVGA_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , SVGA_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (SVGA_LCD_H_BACK_PORCH + SVGA_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (SVGA_LCD_V_BACK_PORCH + SVGA_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , SVGA_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , SVGA_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_lvds_to_hdmi_1024x768 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , XGA_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , XGA_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (XGA_LCD_PIXEL_WIDTH + XGA_LCD_H_FRONT_PORCH + XGA_LCD_H_BACK_PORCH + XGA_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (XGA_LCD_PIXEL_HEIGHT + XGA_LCD_V_FRONT_PORCH + XGA_LCD_V_BACK_PORCH + XGA_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , XGA_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , XGA_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (XGA_LCD_H_BACK_PORCH + XGA_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (XGA_LCD_V_BACK_PORCH + XGA_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , XGA_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , XGA_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

static const DisplayBase::lcd_config_t LcdCfgTbl_lvds_to_hdmi_1280x720 = {
    DisplayBase::LCD_TYPE_PARALLEL_RGB                                              /* lcd_type             */
    , HD_720P_LCD_INPUT_CLOCK                                                               /* intputClock          */
    , HD_720P_LCD_OUTPUT_CLOCK                                                              /* outputClock          */
    , DisplayBase::LCD_OUTFORMAT_RGB888                                             /* lcd_outformat        */
    , DisplayBase::EDGE_FALLING                                                     /* lcd_edge             */
    , (HD_720P_LCD_PIXEL_WIDTH + HD_720P_LCD_H_FRONT_PORCH + HD_720P_LCD_H_BACK_PORCH + HD_720P_LCD_H_SYNC_WIDTH)   /* h_toatal_period      */
    , (HD_720P_LCD_PIXEL_HEIGHT + HD_720P_LCD_V_FRONT_PORCH + HD_720P_LCD_V_BACK_PORCH + HD_720P_LCD_V_SYNC_WIDTH)  /* v_toatal_period      */
    , HD_720P_LCD_PIXEL_WIDTH                                                               /* h_disp_widht         */
    , HD_720P_LCD_PIXEL_HEIGHT                                                              /* v_disp_widht         */
    , (HD_720P_LCD_H_BACK_PORCH + HD_720P_LCD_H_SYNC_WIDTH)                                         /* h_back_porch         */
    , (HD_720P_LCD_V_BACK_PORCH + HD_720P_LCD_V_SYNC_WIDTH)                                         /* v_back_porch         */
    , LCD_H_SYNC_PORT                                                               /* h_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* h_sync_port_polarity */
    , HD_720P_LCD_H_SYNC_WIDTH                                                              /* h_sync_width         */
    , LCD_V_SYNC_PORT                                                               /* v_sync_port          */
    , DisplayBase::SIG_POL_INVERTED                                                 /* v_sync_port_polarity */
    , HD_720P_LCD_V_SYNC_WIDTH                                                              /* v_sync_width         */
    , LCD_DE_PORT                                                                   /* de_port              */
    , DisplayBase::SIG_POL_NOT_INVERTED                                             /* de_port_polarity     */
};

#if defined(TARGET_RZ_A1H)
 #define LCD_CNTRST_PIN    P8_15
 #define LCD_PWON_PIN      P7_15
#elif defined(TARGET_GR_LYCHEE)
 #define LCD_CNTRST_PIN    P3_12
 #define LCD_PWON_PIN      P5_12
#elif defined(TARGET_GR_MANGO)
 #define LCD_CNTRST_PIN    P8_3
 #define LCD_PWON_PIN      P8_1
#elif defined(TARGET_RZ_A2XX)
 #define LCD_CNTRST_PIN    P7_6
 #define LCD_PWON_PIN      NC
#else
#define LCD_CNTRST_PIN    P7_6
#define LCD_PWON_PIN      NC
#endif

#define MAX_BACKLIGHT_VOL       (33.0f)
#ifndef TYPICAL_BACKLIGHT_VOL
  #define TYPICAL_BACKLIGHT_VOL (33.0f)
#endif
// static float voltage_adjust = (TYPICAL_BACKLIGHT_VOL / MAX_BACKLIGHT_VOL);

static void *lcd_port_init(void *display, uint16_t lcd_type, uint16_t lcd_size) {
    DisplayBase *Display = (DisplayBase *)display;
    void *m_LcdCfgTbl_LCD_shield;

    // port configuration
    if (lcd_type == LCD_LVDS) {
        #if defined(TARGET_RZ_A1H)
        PinName lvds_pin[8] = {
            P5_7, P5_6, P5_5, P5_4, P5_3, P5_2, P5_1, P5_0
        };
        #elif defined(TARGET_RZ_A2XX)
        PinName lvds_pin[8] = {
            P4_0, P4_1, P4_2, P4_3, P4_4, P4_5, P4_6, P4_7
        };
        #else
        PinName lvds_pin[8] = {
            P4_0, P4_1, P4_2, P4_3, P4_4, P4_5, P4_6, P4_7
        };
        #endif
        Display->Graphics_Lvds_Port_Init(lvds_pin, 8);
    } else {
        #if defined(TARGET_RZ_A1H)
        PinName lcd_pin[28] = {
            P11_15, P11_14, P11_13, P11_12, P5_7, P5_6, P5_5, P5_4, P5_3, P5_2, P5_1, P5_0,
            P4_7, P4_6, P4_5, P4_4, P10_12, P10_13, P10_14, P10_15, P3_15, P3_14, P3_13,
            P3_12, P3_11, P3_10, P3_9, P3_8
        };
        #elif defined(TARGET_GR_LYCHEE)
        PinName lcd_pin[28] = {
            P6_15, P6_14, P6_13, P6_12, P6_11, P6_10, P6_9, P6_8, P6_7, P6_6, P6_5, P6_4,
            P6_3, P6_2, P6_1, P6_0, P3_7, P3_6, P3_5, P3_4, P3_3, P3_2, P3_1, P3_0, P5_2,
            P5_1, P5_0, P7_4,
        };
        #elif defined(TARGET_GR_MANGO)
        PinName lcd_pin[28] = {
            PB_5, PB_4, PB_3, PB_2, PB_1, PB_0, PA_7, PA_6, PA_5, PA_4, PA_3, PA_2, PA_1, PA_0,
            P8_0, PF_0, PF_1, PF_2, PF_3, PF_4, PF_5, PF_6, PH_2, PF_7,
            P7_2, P7_6, P7_7, PJ_6  /* LCD0_TCON2, LCD0_TCON1, LCD0_TCON0, LCD0_CLK */
        };
        #elif defined(TARGET_RZ_A2XX)
        PinName lcd_pin[28] = {
            PB_5, PB_4, PB_3, PB_2, PB_1, PB_0, PA_7, PA_6, PA_5, PA_4, PA_3, PA_2, PA_1, PA_0,
            P8_0, PF_0, PF_1, PF_2, PF_3, PF_4, PF_5, PF_6, PH_2, PF_7,
            PC_3, PC_4, P7_7, PJ_6  /* LCD0_TCON4, LCD0_TCON3, LCD0_TCON0, LCD0_CLK */
        };
        #else
        PinName lcd_pin[28] = {
            PB_5, PB_4, PB_3, PB_2, PB_1, PB_0, PA_7, PA_6, PA_5, PA_4, PA_3, PA_2, PA_1, PA_0,
            P8_0, PF_0, PF_1, PF_2, PF_3, PF_4, PF_5, PF_6, PH_2, PF_7,
            PC_3, PC_4, P7_7, PJ_6  /* LCD0_TCON4, LCD0_TCON3, LCD0_TCON0, LCD0_CLK */
        };
        #error "MBED_CONF_APP_LCD_TYPE is not supported in this target."
        #endif
        Display->Graphics_Lcd_Port_Init(lcd_pin, 28);
    }

    // lcd parameter configuration
    m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_lvds_to_hdmi_1024x768;
    if (lcd_type == GR_PEACH_DISPLAY_SHIELD) {
        switch (lcd_size) {
            case SVGA:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_Display_shield_800x600;
                break;
            case XGA:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_Display_shield_1024x768;
                break;
            case HD_720p:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_Display_shield_1280x720;
                break;
        }
        return (void *)m_LcdCfgTbl_LCD_shield;
    } else if ((lcd_type & 0x00ff) == TFP410PAP) {
        const char send_cmd[3] = {0x08u, 0xbfu, 0x70u};
        #if defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
        I2C mI2c_(PD_5, PD_4);
        #else
        I2C mI2c_(I2C_SDA, I2C_SCL);
        #endif
        mI2c_.write(0x78, send_cmd, 3);
        return (void *)m_LcdCfgTbl_LCD_shield;
    } else if ((lcd_type & 0x00ff) == EP952) {
        #if defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
        I2C mI2c_(PD_5, PD_4);
        #else
        I2C mI2c_(I2C_SDA, I2C_SCL);
        #endif
        char i2cbuf[20];
        // wait 10ms before turning on ep952
        DigitalOut ep952_rst(PK_5, 0);
        ThisThread::sleep_for(10ms);
        ep952_rst = 1;

        i2cbuf[0] = 0x00;
        i2cbuf[1] = 0x80;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x40;
        i2cbuf[1] = 0x08;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x05;
        i2cbuf[1] = 0x14;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x7B;
        i2cbuf[1] = 0x00;
        i2cbuf[2] = 0x00;
        i2cbuf[3] = 0x00;
        i2cbuf[4] = 0x00;
        i2cbuf[5] = 0x01;
        mI2c_.write(0x52,i2cbuf,6);

        i2cbuf[0] = 0x63;
        i2cbuf[1] = 0x00;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x64;
        i2cbuf[1] = 0x16;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x65;
        i2cbuf[1] = 0x0C;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x74;
        i2cbuf[1] = 0x70;
        i2cbuf[2] = 0x01;
        i2cbuf[3] = 0x00;
        i2cbuf[4] = 0x00;
        i2cbuf[5] = 0x00;
        i2cbuf[6] = 0x00;
        mI2c_.write(0x52,i2cbuf,6);

        i2cbuf[0] = 0x0A;
        i2cbuf[1] = 0x81;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x33;
        i2cbuf[1] = 0x0F;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x0E;
        i2cbuf[1] = 0x0D;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x66;
        i2cbuf[1] = 0xBB;
        i2cbuf[2] = 0x10;
        i2cbuf[3] = 0xA0;
        i2cbuf[4] = 0x00;
        i2cbuf[5] = 0x04;
        i2cbuf[6] = 0x00;
        i2cbuf[7] = 0x00;
        i2cbuf[8] = 0x00;
        i2cbuf[9] = 0x00;
        i2cbuf[10] = 0x00;
        i2cbuf[11] = 0x00;
        i2cbuf[12] = 0x00;
        i2cbuf[13] = 0x00;
        i2cbuf[14] = 0x00;
        mI2c_.write(0x52,i2cbuf,15);

        i2cbuf[0] = 0x3F;
        i2cbuf[1] = 0xF8;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x0D;
        i2cbuf[1] = 0x80;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x0C;
        i2cbuf[1] = 0x30;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x08;
        i2cbuf[1] = 0x97;
        mI2c_.write(0x52,i2cbuf,2);

        i2cbuf[0] = 0x01;
        i2cbuf[1] = 0x00;
        mI2c_.write(0x52,i2cbuf,2);

        switch (lcd_size) {
            case SD_7INCH:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_rgb_to_hdmi_800x480;
                break;
            case SVGA:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_rgb_to_hdmi_800x600;
                break;
            case XGA:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_rgb_to_hdmi_1024x768;
                break;
            case HD_720p:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_rgb_to_hdmi_1280x720;
                break;
        }
    } else if ((lcd_type == GR_PEACH_4_3INCH_SHIELD) || (lcd_type == GR_PEACH_7_1INCH_SHIELD) || (lcd_type == GR_PEACH_RSK_TFT)) {
        static DigitalOut lcd_cntrst(LCD_CNTRST_PIN);
        DigitalOut lcd_pwon(LCD_PWON_PIN);
        DigitalOut lcd_blon(P8_1);
        lcd_pwon = 0;
        lcd_blon = 0;
        ThisThread::sleep_for(100ms);
        lcd_pwon = 1;
        lcd_blon = 1;
        switch (lcd_type) {
            case GR_PEACH_4_3INCH_SHIELD:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_lvds_to_hdmi_800x480;
                break;
            case GR_PEACH_7_1INCH_SHIELD:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_lvds_to_hdmi_800x600;
                break;
            case GR_PEACH_RSK_TFT:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_lvds_to_hdmi_1024x768;
                break;
        }
    } else if (lcd_type == RSK_TFT) {
        static PwmOut lcd_cntrst(LCD_CNTRST_PIN);
        lcd_cntrst.period_us(500);
        m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_RSK_TFT_800x480;
    } else {
        static PwmOut lcd_cntrst(LCD_CNTRST_PIN);
        DigitalOut lcd_pwon(LCD_PWON_PIN);
        lcd_pwon = 0;
        lcd_cntrst.period_us(500);
        ThisThread::sleep_for(100ms);
        lcd_pwon = 1;
        m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_lvds_to_hdmi_1024x768;
        switch (lcd_type) {
            case LCD_800x480:
                if (lcd_size) {
                    m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_40pin_5inch_800x480;
                }
                break;
            case ATM0430D25:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_40pin_4_3inch_480x272;
                break;
            case TF043HV001A0:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_40pin_4_3inch_480x272;
                break;
            case FG040346DSSWBG03:
                m_LcdCfgTbl_LCD_shield = (void *)&LcdCfgTbl_40pin_4_3inch_480x272;
                break;
        }
    }
    return (void *)m_LcdCfgTbl_LCD_shield;
}

void mbed_lcd_start_display(display_t *dp) {
    if (!mbed_lcd_is_initialzed()) {
        return;
    }
    DisplayBase::rect_t rect;
    DisplayBase::graphics_format_t gformat = (DisplayBase::graphics_format_t)dp->format;
    DisplayBase::wr_rd_swa_t wr_rd = (DisplayBase::wr_rd_swa_t)dp->swa;
    dcache_clean((void *)dp->buf, (uint32_t)dp->stride * (uint32_t)dp->width);
    rect.hs = dp->rect.hs;
    rect.hw = dp->rect.hw;
    rect.vs = dp->rect.vs;
    rect.vw = dp->rect.vw;
    g_mbed_display.Graphics_Read_Setting(
        (DisplayBase::graphics_layer_t)dp->layer_id,
        (void *)dp->buf,
        (unsigned int)dp->stride,
        (DisplayBase::graphics_format_t)gformat,
        (DisplayBase::wr_rd_swa_t)wr_rd,
        &rect
        );
    g_mbed_display.Graphics_Start((DisplayBase::graphics_layer_t)dp->layer_id);
    ThisThread::sleep_for(50ms);
    EasyAttach_LcdBacklight(true);
}

void mbed_lcd_init_params(lcd_t *lcd) {
    switch (lcd->lcd_id) {
        case TF043HV001A0:
            lcd->width = 480;
            lcd->height = 272;
            lcd->lcd_size = SD_7INCH;
            break;
        case ATM0430D25:
            lcd->width = 480;
            lcd->height = 272;
            lcd->lcd_size = SD_7INCH;
            break;
        case FG040346DSSWBG03:
            lcd->width = 480;
            lcd->height = 272;
            lcd->lcd_size = SD_7INCH;
            break;
        case LCD_800x480:
            lcd->width = 800;
            lcd->height = 480;
            lcd->lcd_size = SD_7INCH;
            break;
        default:
            lcd->width = 800;
            lcd->height = 480;
            lcd->lcd_size = SD_7INCH;
            break;
    }
}

bool mbed_lcd_graphic_init(lcd_t *lcd) {
    DisplayBase::lcd_config_t *lcd_config = (DisplayBase::lcd_config_t *)lcd_port_init(&g_mbed_display, lcd->lcd_id, lcd->lcd_size);
    DisplayBase::graphics_error_t error = g_mbed_display.Graphics_init(lcd_config);
    if (error != DisplayBase::GRAPHICS_OK) {
        return false;
    } else {
        return true;
    }
}

bool mbed_lcd_init(lcd_t *lcd) {
    mbed_lcd_init_params(lcd);
    bool flag = mbed_lcd_graphic_init(lcd);
    if (flag) {
        m_lcd_initialized = true;
    } else {
        m_lcd_initialized = false;
    }
    return flag;
}

void mbed_lcd_deinit(lcd_t *lcd) {
    m_lcd_initialized = false;
}

bool mbed_lcd_is_initialzed(void) {
    return m_lcd_initialized;
}

bool mbed_lcd_Graphics_Read_Setting(uint16_t layer, uint8_t *buf,
    uint16_t stride, uint16_t format, uint16_t wr_rd, uint16_t hs, uint16_t vs, uint16_t hw, uint16_t vw) {
    DisplayBase::rect_t rect;
    rect.hs = hs;
    rect.vs = vs;
    rect.hw = hw;
    rect.vw = vw;
    DisplayBase::graphics_error_t error = g_mbed_display.Graphics_Read_Setting(
        (DisplayBase::graphics_layer_t)layer,
        (void *)buf,
        (unsigned int)stride,
        (DisplayBase::graphics_format_t)format,
        (DisplayBase::wr_rd_swa_t)wr_rd,
        &rect
        );
    if (error != DisplayBase::GRAPHICS_OK) {
        return false;
    } else {
        return true;
    }
}
