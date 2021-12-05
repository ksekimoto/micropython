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

#ifndef LCD_S1D15G10_H_
#define LCD_S1D15G10_H_

// Epson S1D15G10 Command Set
#define S1D15G10_DISON   0xaf
#define S1D15G10_DISOFF  0xae
#define S1D15G10_DISNOR  0xa6
#define S1D15G10_DISINV  0xa7
#define S1D15G10_COMSCN  0xbb
#define S1D15G10_DISCTL  0xca
#define S1D15G10_SLPIN   0x95
#define S1D15G10_SLPOUT  0x94
#define S1D15G10_PASET   0x75
#define S1D15G10_CASET   0x15
#define S1D15G10_DATCTL  0xbc
#define S1D15G10_RGBSET8 0xce
#define S1D15G10_RAMWR   0x5c
#define S1D15G10_RAMRD   0x5d
#define S1D15G10_PTLIN   0xa8
#define S1D15G10_PTLOUT  0xa9
#define S1D15G10_RMWIN   0xe0
#define S1D15G10_RMWOUT  0xee
#define S1D15G10_ASCSET  0xaa
#define S1D15G10_SCSTART 0xab
#define S1D15G10_OSCON   0xd1
#define S1D15G10_OSCOFF  0xd2
#define S1D15G10_PWRCTR  0x20
#define S1D15G10_VOLCTR  0x81
#define S1D15G10_VOLUP   0xd6
#define S1D15G10_VOLDOWN 0xd7
#define S1D15G10_TMPGRD  0x82
#define S1D15G10_EPCTIN  0xcd
#define S1D15G10_EPCOUT  0xcc
#define S1D15G10_EPMWR   0xfc
#define S1D15G10_EPMRD   0xfd
#define S1D15G10_EPSRRD1 0x7c
#define S1D15G10_EPSRRD2 0x7d
#define S1D15G10_NOP     0x25

#define S1D15G10_PWX          132
#define S1D15G10_PWY          132
#define S1D15G10_SX           2
#define S1D15G10_SY           4
#define S1D15G10_WX           128
#define S1D15G10_WY           128
#define S1D15G10_BITSPERPIXEL 12
#define S1D15G10_FCOL         0xFFFFFF
#define S1D15G10_BCOL         0x000000

#endif /* LCD_S1D15G10_H_ */
