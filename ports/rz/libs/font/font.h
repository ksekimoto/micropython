/*
 * Copyright (c) 2022, Kentaro Sekimoto
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

#ifndef SFONT_H_
#define SFONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MISAKIFONT4X8  1
#define MISAKIFONT6X12 2
#if MICROPY_PY_PYB_UNICODE_FONT
#define MISAKIFONT8X8   3
#define MISAKIFONT12X12 4
#endif

enum
{
    FONT_ASCII = 1,
    FONT_UNICODE = 2,
};

typedef struct _ASCII_FONT_TBL {
    int font_wx;
    int font_wy;
    int font_bytes;
    unsigned char *ascii_font_data;
} ASCII_FONT_TBL;

typedef struct _UNICODE_FONT_TBL {
    int font_wx;
    int font_wy;
    int font_bytes;
    unsigned short *CUniFontIdx;
    unsigned char *CUniFontMap;
    unsigned char *unicode_font_data;
} UNICODE_FONT_TBL;

#define CUNIFONT_TBL_SIZE (0x100)
#define CUNIFONT_ARY_SIZE (0x10000 / (CUNIFONT_TBL_SIZE))

typedef struct _FONT_TBL {
    int font_type;
    int font_unitx;
    int font_unity;
    char *font_name;
    ASCII_FONT_TBL *ascii_font_tbl;
    UNICODE_FONT_TBL *unicode_font_tbl;
} FONT_TBL;

typedef struct {
    char *fontName;
    int fontType;
    int fontUnitX;
    int fontUnitY;
    int fontWidth;
    int fontHeight;
    FONT_TBL *_font_tbl;
} font_t;

extern font_t *fontList[];
extern font_t MisakiFont4x8;
extern font_t MisakiFont8x8;
extern font_t MisakiFont6x12;
extern font_t MisakiFont12x12;

void font_init(font_t *font, FONT_TBL *font_tbl);
void font_deinit();
char *font_fontName(font_t *font);
int font_fontType(font_t *font);
int font_fontUnitX(font_t *font);
int font_fontUnitY(font_t *font);
int font_fontWidth(font_t *font, int c);
int font_fontHeight(font_t *font, int c);
int font_fontBytes(font_t *font, int c);
unsigned char *font_fontData(font_t *font, int idx);
void cnv_u8_to_u16(unsigned char *src, int slen, unsigned char *dst, int dsize, int *dlen);
int get_font_by_name(char *name);
bool find_font_id(int font_id);
bool find_font_idx_by_id(int font_id, int *idx);
font_t *get_font_by_id(int font_id);

#ifdef __cplusplus
}
#endif

#endif /* SFONT_H_ */
