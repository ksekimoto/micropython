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

#ifndef LCD_SSD1331_H_
#define LCD_SSD1331_H_

// SSD1331 Commands
#define SSD1331_CMD_DRAWLINE 0x21      // !< Draw line
#define SSD1331_CMD_DRAWRECT 0x22      // !< Draw rectangle
#define SSD1331_CMD_FILL 0x26          // !< Fill enable/disable
#define SSD1331_CMD_SETCOLUMN 0x15     // !< Set column address
#define SSD1331_CMD_SETROW 0x75        // !< Set row adress
#define SSD1331_CMD_CONTRASTA 0x81     // !< Set contrast for color A
#define SSD1331_CMD_CONTRASTB 0x82     // !< Set contrast for color B
#define SSD1331_CMD_CONTRASTC 0x83     // !< Set contrast for color C
#define SSD1331_CMD_MASTERCURRENT 0x87 // !< Master current control
#define SSD1331_CMD_SETREMAP 0xA0      // !< Set re-map & data format
#define SSD1331_CMD_STARTLINE 0xA1     // !< Set display start line
#define SSD1331_CMD_DISPLAYOFFSET 0xA2 // !< Set display offset
#define SSD1331_CMD_NORMALDISPLAY 0xA4 // !< Set display to normal mode
#define SSD1331_CMD_DISPLAYALLON 0xA5  // !< Set entire display ON
#define SSD1331_CMD_DISPLAYALLOFF 0xA6 // !< Set entire display OFF
#define SSD1331_CMD_INVERTDISPLAY 0xA7 // !< Invert display
#define SSD1331_CMD_SETMULTIPLEX 0xA8  // !< Set multiplex ratio
#define SSD1331_CMD_SETMASTER 0xAD     // !< Set master configuration
#define SSD1331_CMD_DISPLAYOFF 0xAE    // !< Display OFF (sleep mode)
#define SSD1331_CMD_DISPLAYON 0xAF     // !< Normal Brightness Display ON
#define SSD1331_CMD_POWERMODE 0xB0     // !< Power save mode
#define SSD1331_CMD_PRECHARGE 0xB1     // !< Phase 1 and 2 period adjustment
#define SSD1331_CMD_CLOCKDIV   0xB3 // !< Set display clock divide ratio/oscillator frequency
#define SSD1331_CMD_PRECHARGEA 0x8A // !< Set second pre-charge speed for color A
#define SSD1331_CMD_PRECHARGEB 0x8B // !< Set second pre-charge speed for color B
#define SSD1331_CMD_PRECHARGEC 0x8C // !< Set second pre-charge speed for color C
#define SSD1331_CMD_PRECHARGELEVEL 0xBB // !< Set pre-charge voltage
#define SSD1331_CMD_VCOMH 0xBE          // !< Set Vcomh voltge

#endif
