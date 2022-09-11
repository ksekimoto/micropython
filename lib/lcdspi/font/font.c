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
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "mpconfigboard.h"
#include "font.h"

#if defined(USE_DBG_PRINT)
#define DEBUG_LCDSPI
#endif

#ifdef MISAKIFONT4X8
#include "font4x8_data.h"

static const ASCII_FONT_TBL _misaki_font4x8_Tbl = {
    4,
    8,
    8,
    (unsigned char *)misaki_font4x8_data
};

static const FONT_TBL misaki_font4x8_tbl = {
    FONT_ASCII,
    4, /* UnitX */
    8, /* UnitY */
    (char *)"MisakiFont4x8",
    (ASCII_FONT_TBL *)&_misaki_font4x8_Tbl,
    (UNICODE_FONT_TBL *)NULL
};
#endif

#ifdef MISAKIFONT6X12
#include "font6x12_data.h"

static const ASCII_FONT_TBL _misaki_font6x12_Tbl = {
    6,
    12,
    12,
    (unsigned char *)misaki_font6x12_data
};

static const FONT_TBL misaki_font6x12_tbl = {
    FONT_ASCII,
    6,  /* UnitX */
    12, /* UnitY */
    (char *)"MisakiFont6x12",
    (ASCII_FONT_TBL *)&_misaki_font6x12_Tbl,
    (UNICODE_FONT_TBL *)NULL
};
#endif

#ifdef MISAKIFONT8X8
#include "font8x8_data.h"

static const UNICODE_FONT_TBL _misaki_font8x8_Tbl = {
    8,
    8,
    8,
    (unsigned short *)misaki_font8x8_CUniFontIdx,
    (unsigned char *)misaki_font8x8_CUniFontMap,
    (unsigned char *)misaki_font8x8_data
};

static const FONT_TBL misaki_font8x8_tbl = {
    FONT_UNICODE,
    4, /* UnitX */
    8, /* UnitY */
    (char *)"MisakiFont8x8",
    (ASCII_FONT_TBL *)&_misaki_font4x8_Tbl,
    (UNICODE_FONT_TBL *)&_misaki_font8x8_Tbl
};
#endif

#ifdef MISAKIFONT12X12
#include "font12x12_data.h"

static const UNICODE_FONT_TBL _misaki_font12x12_Tbl = {
    12,
    12,
    24,
    (unsigned short *)misaki_font12x12_CUniFontIdx,
    (unsigned char *)misaki_font12x12_CUniFontMap,
    (unsigned char *)misaki_font12x12_data
};

static const FONT_TBL misaki_font12x12_tbl = {
    FONT_UNICODE,
    6,  /* UnitX */
    12, /* UnitY */
    (char *)"MisakiFont12x12",
    (ASCII_FONT_TBL *)&_misaki_font6x12_Tbl,
    (UNICODE_FONT_TBL *)&_misaki_font12x12_Tbl
};
#endif

static const FONT_TBL *fontTblList[] = {
    #ifdef MISAKIFONT4X8
    (FONT_TBL *)&misaki_font4x8_tbl,
    #endif
    #ifdef MISAKIFONT6X12
    (FONT_TBL *)&misaki_font6x12_tbl,
    #endif
    #ifdef MISAKIFONT8X8
    (FONT_TBL *)&misaki_font8x8_tbl,
    #endif
    #ifdef MISAKIFONT12X12
    (FONT_TBL *)&misaki_font12x12_tbl,
    #endif
    (FONT_TBL *)NULL
};

/*
 *  char *fontName;
 *  int fontType;
 *  int fontUnitX;
 *  int fontUnitY;
 *  int fontWidth;
 *  int fontHeight;
 *  int fontBytes;
 *  FONT_TBL *_font_tbl;
 */

#ifdef MISAKIFONT4X8
font_t MisakiFont4x8 = {
    "MisakiFont4x8",
    FONT_ASCII,
    4,
    8,
    4,
    8,
    (FONT_TBL *)&misaki_font4x8_tbl,
};
#endif
#ifdef MISAKIFONT6X12
font_t MisakiFont6x12 = {
    "MisakiFont6x12",
    FONT_ASCII,
    6,
    12,
    6,
    12,
    (FONT_TBL *)&misaki_font6x12_tbl,
};
#endif
#ifdef MISAKIFONT8X8
font_t MisakiFont8x8 = {
    "MisakiFont8x8",
    FONT_UNICODE,
    8,
    8,
    8,
    8,
    (FONT_TBL *)&misaki_font8x8_tbl,
};
#endif
#ifdef MISAKIFONT12X12
font_t MisakiFont12x12 = {
    "MisakiFont12x12",
    FONT_UNICODE,
    12,
    12,
    12,
    12,
    (FONT_TBL *)&misaki_font12x12_tbl,
};
#endif

font_t *fontList[] = {
    #ifdef MISAKIFONT4X8
    (font_t *)&MisakiFont4x8,
    #endif
    #ifdef MISAKIFONT6X12
    (font_t *)&MisakiFont6x12,
    #endif
    #ifdef MISAKIFONT8X8
    (font_t *)&MisakiFont8x8,
    #endif
    #ifdef MISAKIFONT12X12
    (font_t *)&MisakiFont12x12,
    #endif
};

typedef struct _pyb_font_obj_t {
    uint32_t font_id;
    const font_t *font;
} pyb_font_obj_t;

static const pyb_font_obj_t pyb_font_obj[] = {
    #ifdef MISAKIFONT4X8
    { MISAKIFONT4X8, (const font_t *)&MisakiFont4x8 },
    #endif
    #ifdef MISAKIFONT8X8
    { MISAKIFONT8X8, (const font_t *)&MisakiFont8x8 },
    #endif
    #ifdef MISAKIFONT6X12
    { MISAKIFONT6X12, (const font_t *)&MisakiFont6x12 },
    #endif
    #ifdef MISAKIFONT12X12
    { MISAKIFONT12X12, (const font_t *)&MisakiFont12x12 },
    #endif
};
#define NUM_FONTS (sizeof(pyb_font_obj) / sizeof(pyb_font_obj_t))

void font_init(font_t *font, FONT_TBL *font_tbl) {
    font->_font_tbl = font_tbl;
}

void font_deinit() {
}

char *font_fontName(font_t *font) {
    return font->_font_tbl->font_name;
}

int font_fontType(font_t *font) {
    return font->_font_tbl->font_type;
}

int font_fontUnitX(font_t *font) {
    return font->_font_tbl->font_unitx;
}

int font_fontUnitY(font_t *font) {
    return font->_font_tbl->font_unity;
}

int font_fontWidth(font_t *font, int c) {
    if (c < 0x100) {
        return font->_font_tbl->ascii_font_tbl->font_wx;
    } else {
        return font->_font_tbl->unicode_font_tbl->font_wx;
    }
}

int font_fontHeight(font_t *font, int c) {
    if (c < 0x100) {
        return font->_font_tbl->ascii_font_tbl->font_wy;
    } else {
        return font->_font_tbl->unicode_font_tbl->font_wy;
    }
}

int font_fontBytes(font_t *font, int c) {
    if (c < 0x100) {
        return font->_font_tbl->ascii_font_tbl->font_bytes;
    } else {
        return font->_font_tbl->unicode_font_tbl->font_bytes;
    }
}

/*
 * get font data
 */
unsigned char *font_fontData(font_t *font, int idx) {
    unsigned char *p;
    if (font->_font_tbl->font_type == FONT_ASCII) {
        idx &= 0xff;
    }
    if (idx < 0x100) {
        idx &= 0xff;
        #if defined(DEBUG_LCDSPI)
        debug_printf("font8 idx: ", idx);
        #endif
        p = font->_font_tbl->ascii_font_tbl->ascii_font_data;
        p += (idx * font_fontBytes(font, idx));
        return p;
    } else {
        #if defined(DEBUG_LCDSPI)
        debug_printf("font16 idx: ", idx);
        #endif
        int i;
        int fidx;
        int tblH = idx / CUNIFONT_TBL_SIZE;
        int tblL = idx % CUNIFONT_TBL_SIZE;
        unsigned char mask = (unsigned char)(1 << (idx & 7));
        unsigned char *font_map = font->_font_tbl->unicode_font_tbl->CUniFontMap;
        unsigned short *font_idx = font->_font_tbl->unicode_font_tbl->CUniFontIdx;
        int off = (tblH * CUNIFONT_TBL_SIZE) / 8 + tblL / 8;
        if (font_map[off] & mask) {
            fidx = font_idx[tblH];
            for (i = 0; i < tblL; i++) {
                mask = (uint8_t)(1 << (i & 7));
                if (font_map[(tblH * CUNIFONT_TBL_SIZE) / 8 + (i / 8)] & mask) {
                    fidx++;
                }
            }
            #if defined(DEBUG_LCDSPI)
            debug_printf("font16 fidx: ", fidx);
            #endif
            p = font->_font_tbl->unicode_font_tbl->unicode_font_data;
            p += (fidx * font_fontBytes(font, idx));
        } else {
            #if defined(DEBUG_LCDSPI)
            debug_printf("font16 fidx: ", -1);
            #endif
            // space fidx = 228
            // p = (unsigned char *)NULL;
            fidx = 228;
            p = font->_font_tbl->unicode_font_tbl->unicode_font_data;
            p += (fidx * font_fontBytes(font, idx));
        }
        return p;
    }
}

/*
 * convert utf8 string to unicode string
 */
void cnv_u8_to_u16(unsigned char *src, int slen, unsigned char *dst, int dsize, int *dlen) {
    int len;
    int idst = 0;
    unsigned char c;
    unsigned int u = 0;
    unsigned short *udst = (unsigned short *)dst;

    while ((slen > 0) && (idst < dsize)) {
        len = 0;
        c = *src++;
        slen--;
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
        while (len-- > 0 && ((c = *src) & 0xC0) == 0x80) {
            u = (u << 6) | (unsigned int)(c & 0x3F);
            src++;
            slen--;
        }
        #if defined(DEBUG_LCDSPI)
        debug_printf("unicode: %x\r\n", u);
        #endif
        if ((0x10000 <= u) && (u <= 0x10FFFF)) {
            if (udst != NULL) {
                udst[idst] = (unsigned short)(0xD800 | (((u & 0x1FFC00) >> 10) - 0x40));
                udst[idst + 1] = (unsigned short)(0xDC00 | (u & 0x3FF));
            }
            idst += 2;
        } else {
            if (udst != NULL) {
                udst[idst] = (uint16_t)u;
            }
            idst++;
        }
    }
    #if defined(DEBUG_LCDSPI)
    debug_printf("len: %d\r\n", idst);
    #endif
    *dlen = idst;
}

int get_font_by_name(char *name) {
    int idx = 0;
    FONT_TBL *p = (FONT_TBL *)fontTblList;
    while ((p != NULL) && (p->font_name != NULL)) {
        if (strcmp(p->font_name, name) == 0) {
            return idx;
        }
        idx++;
    }
    return -1;
}

bool find_font_id(int font_id) {
    bool find = false;
    for (uint32_t i = 0; i < NUM_FONTS; i++) {
        if (pyb_font_obj[i].font_id == (uint32_t)font_id) {
            find = true;
            break;
        }
    }
    return find;
}
