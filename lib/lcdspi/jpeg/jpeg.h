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
#include <stdint.h>
#include <stdbool.h>
#include "picojpeg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
    pjpeg_image_info_t image_info;
    bool is_available;
    bool reduce;    // In reduce mode output 1 pixel per 8x8 block.
    uint32_t err;
    uint16_t mcu_x;
    uint16_t mcu_y;
    uint16_t row_pitch;
    uint16_t row_blocks_per_mcu;
    uint16_t col_blocks_per_mcu;
    uint8_t *pImage;
    uint16_t decoded_width;
    uint16_t decoded_height;
    uint32_t comps;
    uint16_t MCUSPerRow;
    uint16_t MCUSPerCol;
    uint16_t MCUx;
    uint16_t MCUy;
} jpeg_t;

void jpeg_init(jpeg_t *jpeg);
void jpeg_deinit(jpeg_t *jpeg);
bool jpeg_decode(jpeg_t *jpeg, bool reduce);
bool jpeg_decode_mcu(jpeg_t *jpeg);
bool jpeg_read(jpeg_t *jpeg);

#ifdef __cplusplus
}
#endif

#endif
