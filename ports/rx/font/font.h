/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Kentaro Sekimoto
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

#ifndef SFONT_H_
#define SFONT_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
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
#define CUNIFONT_ARY_SIZE (0x10000/(CUNIFONT_TBL_SIZE))

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

extern const mp_obj_type_t pyb_font_type;
extern font_t *fontList[];
extern font_t MisakiFont4x8;
extern font_t MisakiFont8x8;
extern font_t MisakiFont6x12;
extern font_t MisakiFont12x12;

int font_fontUnitX(font_t *font);
int font_fontUnitY(font_t *font);
int font_fontWidth(font_t *font, int c);
int font_fontHeight(font_t *font, int c);
int font_fontBytes(font_t *font, int c);
unsigned char *font_fontData(font_t *font, int idx);

#ifdef __cplusplus
}
#endif

#endif /* SFONT_H_ */
