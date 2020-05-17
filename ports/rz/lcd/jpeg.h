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

#ifndef SJPEG_H
#define SJPEG_H

#include <stdio.h>
#include "picojpeg.h"

#ifndef max
#define max(a,b)     (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)     (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
    pjpeg_image_info_t image_info;
    int is_available;
    int mcu_x;
    int mcu_y;
    uint32_t row_pitch;
    uint32_t row_blocks_per_mcu;
    uint32_t col_blocks_per_mcu;
    int m_split;
    uint8_t *pImage;
    int err;
    int decoded_width;
    int decoded_height;
    int comps;
    int MCUSPerRow;
    int MCUSPerCol;
    pjpeg_scan_type_t scanType;
    int MCUx;
    int MCUy;
} jpeg_t;

extern jpeg_t jpeg;

void jpeg_init(jpeg_t *jpeg);
void jpeg_deinit(jpeg_t *jpeg);
int jpeg_decode(jpeg_t *jpeg, char *filename, int split);
int jpeg_decode_mcu(jpeg_t *jpeg);
int jpeg_read(jpeg_t *jpeg);

#endif
