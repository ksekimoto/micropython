/*
 * Copyright (c) 2021, Kentaro Sekimoto
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

#ifndef LCD_ILI9488_H_
#define LCD_ILI9488_H_

#include "lcdspi.h"

// ILI9488
#define ILI9488_NOP      0x00  // nop
#define ILI9488_SWRESET  0x01  // software reset
#define ILI9488_RDDIDIF  0x04  // read display identification
#define ILI9488_RDDST    0x09  // read display status
#define ILI9488_SLEEPIN  0x10  // sleep in
#define ILI9488_SLEEPOUT 0x11  // sleep out
#define ILI9488_PTLON    0x12  // partial display mode
#define ILI9488_NORON    0x13  // display normal mode
#define ILI9488_INVOFF   0x20  // inversion OFF
#define ILI9488_INVON    0x21  // inversion ON
#define ILI9488_DALO     0x22  // all pixel OFF
#define ILI9488_DAL      0x23  // all pixel ON
#define ILI9488_DISPOFF  0x28  // display OFF
#define ILI9488_DISPON   0x29  // display ON
#define ILI9488_CASET    0x2A  // column address set
#define ILI9488_PASET    0x2B  // page address set
#define ILI9488_RAMWR    0x2C  // memory write
#define ILI9488_RGBSET   0x2D  // colour set
#define ILI9488_PTLAR    0x30  // partial area
#define ILI9488_VSCRDEF  0x33  // vertical scrolling definition
#define ILI9488_TEOFF    0x34  // test mode
#define ILI9488_TEON     0x35  // test mode
#define ILI9488_MADCTL   0x36  // memory access control
#define ILI9488_SEP      0x37  // vertical scrolling start address
#define ILI9488_IDMOFF   0x38  // idle mode OFF
#define ILI9488_IDMON    0x39  // idle mode ON
#define ILI9488_COLMOD   0x3A  // interface pixel format
#define ILI9488_RDID1    0xDA  // read ID1
#define ILI9488_RDID2    0xDB  // read ID2
#define ILI9488_RDID3    0xDC  // read ID3

#endif /* LCD_ILI9488_H_ */
