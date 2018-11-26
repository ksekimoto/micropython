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

#include <stdio.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"

#define MISAKIFONT4X8
#define MISAKIFONT6X12
#define MISAKIFONT8X8
#define MISAKIFONT12X12

#include "font.h"

//#define   DEBUG       // Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

#ifdef MISAKIFONT4X8
#include "font4x8_data.h"

static const ASCII_FONT_TBL _misaki_font4x8_Tbl = {
    4,
    8,
    8,
    (unsigned char *)misaki_font4x8_data };

static const FONT_TBL misaki_font4x8_tbl = {
    FONT_ASCII,
    4, /* UnitX */
    8, /* UnitY */
    (char *)"MisakiFont4x8",
    (ASCII_FONT_TBL *)&_misaki_font4x8_Tbl,
    (UNICODE_FONT_TBL *)NULL };
#endif

#ifdef MISAKIFONT6X12
#include "font6x12_data.h"

static const ASCII_FONT_TBL _misaki_font6x12_Tbl = {
    6,
    12,
    12,
    (unsigned char *)misaki_font6x12_data };

static const FONT_TBL misaki_font6x12_tbl = {
    FONT_ASCII,
    6, /* UnitX */
    12, /* UnitY */
    (char *)"MisakiFont6x12",
    (ASCII_FONT_TBL *)&_misaki_font6x12_Tbl,
    (UNICODE_FONT_TBL *)NULL };
#endif

#ifdef MISAKIFONT8X8
#include "font8x8_data.h"

static const UNICODE_FONT_TBL _misaki_font8x8_Tbl = {
    8,
    8,
    8,
    (unsigned short *)misaki_font8x8_CUniFontIdx,
    (unsigned char *)misaki_font8x8_CUniFontMap,
    (unsigned char *)misaki_font8x8_data };

static const FONT_TBL misaki_font8x8_tbl = {
    FONT_UNICODE,
    4, /* UnitX */
    8, /* UnitY */
    (char *)"MisakiFont8x8",
    (ASCII_FONT_TBL *)&_misaki_font4x8_Tbl,
    (UNICODE_FONT_TBL *)&_misaki_font8x8_Tbl };
#endif

#ifdef MISAKIFONT12X12
#include "font12x12_data.h"

static const UNICODE_FONT_TBL _misaki_font12x12_Tbl = {
    12,
    12,
    24,
    (unsigned short *)misaki_font12x12_CUniFontIdx,
    (unsigned char *)misaki_font12x12_CUniFontMap,
    (unsigned char *)misaki_font12x12_data };

static const FONT_TBL misaki_font12x12_tbl = {
    FONT_UNICODE,
    6, /* UnitX */
    12, /* UnitY */
    (char *)"MisakiFont12x12",
    (ASCII_FONT_TBL *)&_misaki_font6x12_Tbl,
    (UNICODE_FONT_TBL *)&_misaki_font12x12_Tbl };
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
    4, 8, 4, 8,
    (FONT_TBL *)&misaki_font4x8_tbl,
};
#endif
#ifdef MISAKIFONT6X12
font_t MisakiFont6x12 = {
    "MisakiFont6x12",
    FONT_ASCII,
    6, 12, 6, 12,
    (FONT_TBL *)&misaki_font6x12_tbl,
};
#endif
#ifdef MISAKIFONT8X8
font_t MisakiFont8x8 = {
    "MisakiFont8x8",
    FONT_UNICODE,
    8, 8, 8, 8,
    (FONT_TBL *)&misaki_font8x8_tbl,
};
#endif
#ifdef MISAKIFONT12X12
font_t MisakiFont12x12 = {
    "MisakiFont12x12",
    FONT_UNICODE,
    12, 12, 12, 12,
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
    mp_obj_base_t base;
    mp_uint_t font_id;
    const font_t *font;
} pyb_font_obj_t;

STATIC const pyb_font_obj_t pyb_font_obj[] = {
#ifdef MISAKIFONT4X8
    {{&pyb_font_type}, 0, (const font_t *)&MisakiFont4x8},
#endif
#ifdef MISAKIFONT8X8
    {{&pyb_font_type}, 1, (const font_t *)&MisakiFont8x8},
#endif
#ifdef MISAKIFONT6X12
    {{&pyb_font_type}, 2, (const font_t *)&MisakiFont6x12},
#endif
#ifdef MISAKIFONT12X12
    {{&pyb_font_type}, 3, (const font_t *)&MisakiFont12x12},
#endif
};
#define NUM_FONTS   MP_ARRAY_SIZE(pyb_font_obj)

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
    if (c < 0x100)
        return font->_font_tbl->ascii_font_tbl->font_wx;
    else
        return font->_font_tbl->unicode_font_tbl->font_wx;
}

int font_fontHeight(font_t *font, int c) {
    if (c < 0x100)
        return font->_font_tbl->ascii_font_tbl->font_wy;
    else
        return font->_font_tbl->unicode_font_tbl->font_wy;
}

int font_fontBytes(font_t *font, int c) {
    if (c < 0x100)
        return font->_font_tbl->ascii_font_tbl->font_bytes;
    else
        return font->_font_tbl->unicode_font_tbl->font_bytes;
}

/*
 * get font data
 */
unsigned char *font_fontData(font_t *font, int idx) {
    unsigned char *p;
    if (idx < 0x100) {
        idx &= 0xff;
        DEBUG_PRINT("font8 idx: ", idx);
        p = font->_font_tbl->ascii_font_tbl->ascii_font_data;
        p += (idx * font_fontBytes(font, idx));
        return p;
    } else {
        DEBUG_PRINT("font16 idx: ", idx);
        int i;
        int fidx;
        int tblH = idx / CUNIFONT_TBL_SIZE;
        int tblL = idx % CUNIFONT_TBL_SIZE;
        unsigned char mask = (unsigned char)(1 << (idx & 7));
        unsigned char *font_map = font->_font_tbl->unicode_font_tbl->CUniFontMap;
        unsigned short *font_idx = font->_font_tbl->unicode_font_tbl->CUniFontIdx;
        if (font_map[(tblH * CUNIFONT_TBL_SIZE) / 8 + tblL / 8] & mask) {
            fidx = font_idx[tblH];
            for (i = 0; i < tblL; i++) {
                mask = (1 << (i & 7));
                if (font_map[(tblH * CUNIFONT_TBL_SIZE) / 8 + (i / 8)] & mask) {
                    fidx++;
                }
            } DEBUG_PRINT("font16 fidx: ", fidx);
            p = font->_font_tbl->unicode_font_tbl->unicode_font_data;
            p += (fidx * font_fontBytes(font, idx));
        } else {
            DEBUG_PRINT("font16 fidx: ", -1);
            p = (unsigned char *) NULL;
        }
        return p;
    }
}

/*
 * convert utf8 string to unicode string
 */
static void cnv_u8_to_u16(unsigned char *src, int slen, unsigned char *dst, int dsize, int *dlen) {
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
        DEBUG_PRINT("unicode",u)
        if ((0x10000 <= u) && (u <= 0x10FFFF)) {
            if (udst != NULL) {
                udst[idst] = (unsigned short)(0xD800 | (((u & 0x1FFC00) >> 10) - 0x40));
                udst[idst + 1] = (unsigned short)(0xDC00 | (u & 0x3FF));
            }
            idst += 2;
        } else {
            if (udst != NULL) {
                udst[idst] = u;
            }
            idst++;
        }
    }
    DEBUG_PRINT("len", idst)
    *dlen = idst;
}

int get_font_by_name(char *name) {
    int idx = 0;
    FONT_TBL *p = (FONT_TBL *)fontTblList;
    while (p != NULL) {
        if (strcmp(p->font_name, name) == 0) {
            return idx;
        }
        idx++;
    }
    return -1;
}

bool find_font_id(int font_id) {
    bool find = false;
    for (int i = 0; i < NUM_FONTS; i++) {
        if (pyb_font_obj[i].font_id == font_id) {
            find = true;
            break;
        }
    }
    return find;
}

font_t *get_font_by_id(int font_id) {
    font_t *font = 0;
    for (int i = 0; i < NUM_FONTS; i++) {
        if (pyb_font_obj[i].font_id == font_id) {
            font = pyb_font_obj[i].font;
            break;
        }
    }
    return font;
}

/******************************************************************************/
/* MicroPython bindings                                                       */

void font_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "FONT(%u):%s", self->font_id, self->font->fontName);
}

STATIC mp_obj_t pyb_font_name(mp_obj_t self_in) {
    pyb_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char *font_name = self->font->fontName;
    return mp_obj_new_str((const char *)font_name, strlen(font_name));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_font_name_obj, pyb_font_name);

STATIC mp_obj_t pyb_font_width(mp_obj_t self_in) {
    pyb_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_width = self->font->fontWidth;
    return mp_obj_new_int(font_width);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_font_width_obj, pyb_font_width);

STATIC mp_obj_t pyb_font_height(mp_obj_t self_in) {
    pyb_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_height = self->font->fontHeight;
    return mp_obj_new_int(font_height);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_font_height_obj, pyb_font_height);

STATIC mp_obj_t pyb_font_data(mp_obj_t self_in, mp_obj_t idx) {
    pyb_font_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int font_idx = mp_obj_get_int(idx);
    int font_bytes = font_fontBytes(self->font, font_idx);
    unsigned char *font_data = font_fontData(self->font, font_idx);
    return mp_obj_new_bytearray_by_ref((size_t)font_bytes, (void *)font_data);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_font_data_obj, pyb_font_data);

/// \classmethod \constructor(id)
/// Create an FONT object
///
STATIC mp_obj_t font_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    mp_int_t font_id = mp_obj_get_int(args[0]);
    // check font number
    if (!find_font_id(font_id)) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "FONT(%d) doesn't exist", font_id));
    }
    // return static font object
    return MP_OBJ_FROM_PTR(&pyb_font_obj[font_id]);
}

STATIC const mp_rom_map_elem_t font_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_name), MP_ROM_PTR(&pyb_font_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&pyb_font_width_obj) },
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&pyb_font_height_obj) },
    { MP_ROM_QSTR(MP_QSTR_data), MP_ROM_PTR(&pyb_font_data_obj) },
};

STATIC MP_DEFINE_CONST_DICT(font_locals_dict, font_locals_dict_table);

const mp_obj_type_t pyb_font_type = {
    { &mp_type_type },
    .name = MP_QSTR_FONT,
    .print = font_obj_print,
    .make_new = font_obj_make_new,
    .locals_dict = (mp_obj_dict_t*)&font_locals_dict,
};


#if 0
//**************************************************
// メモリの開放時に走る
//**************************************************
static void font_free(mrb_state *mrb, void *ptr) {
	Font* font = static_cast<Font*>(ptr);
	delete font;
}

//**************************************************
// この構造体の意味はよくわかっていない
//**************************************************
static struct mrb_data_type font_type = { "Font", font_free };

mrb_value mrb_font_initialize(mrb_state *mrb, mrb_value self)
{
	DATA_TYPE(self) = &font_type;
	DATA_PTR(self) = NULL;
	int font_idx;
	mrb_get_args(mrb, "i", &font_idx);
	if (font_idx < 0)
		font_idx = 0;
	Font *font = fontList[font_idx];
	DEBUG_PRINT("mrb_font_initialize font_idx", font_idx);
	DATA_PTR(self) = font;
	return self;
}

mrb_value mrb_font_data(mrb_state *mrb, mrb_value self)
{
	Font* font = static_cast<Font*>(mrb_get_datatype(mrb, self, &font_type));
	unsigned char *buf;
	int idx;
	mrb_get_args(mrb, "i", &idx);
	buf = (unsigned char *)font->fontData(idx);
	DEBUG_PRINT("mrb_font_data buf", (int)buf);
	return mrb_str_new(mrb, (const char*)buf, font->fontBytes(idx));
}

#if 0
mrb_value mrb_font_cnvUtf8ToUnicode(mrb_state *mrb, mrb_value self)
{
	mrb_value v;
	mrb_value vsrc;
	char *src;
	int slen;
	int size = 0;
	mrb_get_args(mrb, "Si", &vsrc, &slen);
	src = RSTRING_PTR(vsrc);
	DEBUG_PRINT("mrb_font_cnvUtf8ToUnicode src len", slen);
	cnv_u8_to_u16((unsigned char *)src, slen, (unsigned char *)NULL, 256, &size);
	unsigned char *u16 = (unsigned char *)malloc(sizeof(unsigned short)*(size+2));
	if (u16) {
		cnv_u8_to_u16((unsigned char *)src, slen, u16, sizeof(unsigned short)*(size+2), &size);
		v = mrb_str_new(mrb, (const char*)u16, size*2);
		free(u16);
	} else {
		v = mrb_str_new(mrb, (const char*)"*", 1);
	}
	return v;
}
#else
#define TMP_BUF_MAX	128
static unsigned char tmp[TMP_BUF_MAX];
mrb_value mrb_font_cnvUtf8ToUnicode(mrb_state *mrb, mrb_value self)
{
	mrb_value v;
	mrb_value vsrc;
	char *src;
	int slen;
	int size = 0;
	mrb_get_args(mrb, "Si", &vsrc, &slen);
	src = RSTRING_PTR(vsrc);
	DEBUG_PRINT("mrb_font_cnvUtf8ToUnicode src len", slen);
	cnv_u8_to_u16((unsigned char *)src, slen, (unsigned char *)tmp, TMP_BUF_MAX, &size);
	v = mrb_str_new(mrb, (const char*)tmp, size*2);
	return v;
}
#endif

mrb_value mrb_font_getUnicodeAtIndex(mrb_state *mrb, mrb_value self)
{
	mrb_value vsrc;
	unsigned char *src;
	int index;
	unsigned int u;
	mrb_get_args(mrb, "Si", &vsrc, &index);
	src = (unsigned char *)RSTRING_PTR(vsrc);
	index *= 2;
	u = ((unsigned int)src[index+1] << 8) + (unsigned int)src[index];
	return mrb_fixnum_value(u);
}

void font_Init(mrb_state *mrb)
{
	//クラスを作成する前には、強制gcを入れる
	mrb_full_gc(mrb);

	struct RClass *fontModule = mrb_define_class(mrb, "Font", mrb->object_class);
	MRB_SET_INSTANCE_TT(fontModule, MRB_TT_DATA);

	mrb_define_method(mrb, fontModule, "initialize", mrb_font_initialize, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, fontModule, "data", mrb_font_data, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, fontModule, "cnvUtf8ToUnicode", mrb_font_cnvUtf8ToUnicode, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, fontModule, "getUnicodeAtIndex", mrb_font_getUnicodeAtIndex, MRB_ARGS_REQ(2));
}

#endif
