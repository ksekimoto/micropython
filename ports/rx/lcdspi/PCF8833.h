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

#ifndef LCD_PCF8833_H_
#define LCD_PCF8833_H_

#include "lcdspi.h"

// PCF8833
#define PCF8833_NOP 0x00        // nop
#define PCF8833_SWRESET 0x01    // software reset
#define PCF8833_BSTROFF 0x02    // booster voltage OFF
#define PCF8833_BSTRON 0x03     // booster voltage ON
#define PCF8833_RDDIDIF 0x04    // read display identification
#define PCF8833_RDDST 0x09      // read display status
#define PCF8833_SLEEPIN 0x10    // sleep in
#define PCF8833_SLEEPOUT 0x11   // sleep out
#define PCF8833_PTLON 0x12      // partial display mode
#define PCF8833_NORON 0x13      // display normal mode
#define PCF8833_INVOFF 0x20     // inversion OFF
#define PCF8833_INVON 0x21      // inversion ON
#define PCF8833_DALO 0x22       // all pixel OFF
#define PCF8833_DAL 0x23        // all pixel ON
#define PCF8833_SETCON 0x25     // write contrast
#define PCF8833_DISPOFF 0x28    // display OFF
#define PCF8833_DISPON 0x29     // display ON
#define PCF8833_CASET 0x2A      // column address set
#define PCF8833_PASET 0x2B      // page address set
#define PCF8833_RAMWR 0x2C      // memory write
#define PCF8833_RGBSET 0x2D     // colour set
#define PCF8833_PTLAR 0x30      // partial area
#define PCF8833_VSCRDEF 0x33    // vertical scrolling definition
#define PCF8833_TEOFF 0x34      // test mode
#define PCF8833_TEON 0x35       // test mode
#define PCF8833_MADCTL 0x36     // memory access control
#define PCF8833_SEP 0x37        // vertical scrolling start address
#define PCF8833_IDMOFF 0x38     // idle mode OFF
#define PCF8833_IDMON 0x39      // idle mode ON
#define PCF8833_COLMOD 0x3A     // interface pixel format
#define PCF8833_SETVOP 0xB0     // set Vop
#define PCF8833_BRS 0xB4        // bottom row swap
#define PCF8833_TRS 0xB6        // top row swap
#define PCF8833_DISCTR 0xB9     // display control
#define PCF8833_DOR 0xBA        // data order
#define PCF8833_TCDFE 0xBD      // enable/disable DF temperature compensation
#define PCF8833_TCVOPE 0xBF     // enable/disable Vop temp comp
#define PCF8833_EC 0xC0         // internal or external oscillator
#define PCF8833_SETMUL 0xC2     // set multiplication factor
#define PCF8833_TCVOPAB 0xC3    // set TCVOP slopes A and B
#define PCF8833_TCVOPCD 0xC4    // set TCVOP slopes c and d
#define PCF8833_TCDF 0xC5       // set divider frequency
#define PCF8833_DF8COLOR 0xC6   // set divider frequency 8-color mode
#define PCF8833_SETBS 0xC7      // set bias system
#define PCF8833_RDTEMP 0xC8     // temperature read back
#define PCF8833_NLI 0xC9        // n-line inversion
#define PCF8833_RDID1 0xDA      // read ID1
#define PCF8833_RDID2 0xDB      // read ID2
#define PCF8833_RDID3 0xDC      // read ID3

#define PCF8833_PWX		132
#define PCF8833_PWY		132
#define PCF8833_SX		2
#define PCF8833_SY		4
#define PCF8833_WX		128
#define PCF8833_WY		128
#define PCF8833_BITSPERPIXEL	12
#define PCF8833_FCOL	0xFFFFFF
#define PCF8833_BCOL	0x000000

#endif /* LCD_PCF8833_H_ */
