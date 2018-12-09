////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2011 Kentaro Sekimoto  All rights reserved.
////////////////////////////////////////////////////////////////////////////

#ifndef ST7735_H_
#define ST7735_H_

#define ST7735_NOP 0x00        // nop
#define ST7735_SWRESET 0x01    // software reset
#define ST7735_BSTROFF 0x02    // booster voltage OFF
#define ST7735_BSTRON 0x03     // booster voltage ON
#define ST7735_RDDIDIF 0x04    // read display identification
#define ST7735_RDDST 0x09      // read display status
#define ST7735_SLEEPIN 0x10    // sleep in
#define ST7735_SLEEPOUT 0x11   // sleep out
#define ST7735_PTLON 0x12      // partial display mode
#define ST7735_NORON 0x13      // display normal mode
#define ST7735_INVOFF 0x20     // inversion OFF
#define ST7735_INVON 0x21      // inversion ON
#define ST7735_DALO 0x22       // all pixel OFF
#define ST7735_DAL 0x23        // all pixel ON
#define ST7735_SETCON 0x25     // write contrast
#define ST7735_DISPOFF 0x28    // display OFF
#define ST7735_DISPON 0x29     // display ON
#define ST7735_CASET 0x2A      // column address set
#define ST7735_PASET 0x2B      // page address set
#define ST7735_RAMWR 0x2C      // memory write
#define ST7735_RGBSET 0x2D     // colour set
#define ST7735_PTLAR 0x30      // partial area
#define ST7735_VSCRDEF 0x33    // vertical scrolling definition
#define ST7735_TEOFF 0x34      // test mode
#define ST7735_TEON 0x35       // test mode
#define ST7735_MADCTL 0x36     // memory access control
#define ST7735_SEP 0x37        // vertical scrolling start address
#define ST7735_IDMOFF 0x38     // idle mode OFF
#define ST7735_IDMON 0x39      // idle mode ON
#define ST7735_COLMOD 0x3A     // interface pixel format
#define ST7735_SETVOP 0xB0     // set Vop
#define ST7735_BRS 0xB4        // bottom row swap
#define ST7735_TRS 0xB6        // top row swap
#define ST7735_DISCTR 0xB9     // display control
#define ST7735_DOR 0xBA        // data order
#define ST7735_TCDFE 0xBD      // enable/disable DF temperature compensation
#define ST7735_TCVOPE 0xBF     // enable/disable Vop temp comp
#define ST7735_EC 0xC0         // internal or external oscillator
#define ST7735_SETMUL 0xC2     // set multiplication factor
#define ST7735_TCVOPAB 0xC3    // set TCVOP slopes A and B
#define ST7735_TCVOPCD 0xC4    // set TCVOP slopes c and d
#define ST7735_TCDF 0xC5       // set divider frequency
#define ST7735_DF8COLOR 0xC6   // set divider frequency 8-color mode
#define ST7735_SETBS 0xC7      // set bias system
#define ST7735_RDTEMP 0xC8     // temperature read back
#define ST7735_NLI 0xC9        // n-line inversion
#define ST7735_RDID1 0xDA      // read ID1
#define ST7735_RDID2 0xDB      // read ID2
#define ST7735_RDID3 0xDC      // read ID3

#endif /* ST7735_H_ */
