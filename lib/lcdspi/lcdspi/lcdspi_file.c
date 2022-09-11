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
#include "mpy_file.h"
#include "lcdspi.h"

#if MICROPY_HW_ENABLE_LCDSPI

static int BYTEARRAY4_TO_INT(uint8_t *a) {
    // return (int)*(uint32_t *)a;
    return (int)((uint32_t)a[0] +
        ((uint32_t)a[1] << 8) +
        ((uint32_t)a[2] << 16) +
        ((uint32_t)a[3] << 24)
        );
}

static int BYTEARRAY2_TO_INT(uint8_t *a) {
    // return (int)*(uint16_t *)a;
    return (int)((uint32_t)a[0] +
        ((uint32_t)a[1] << 8)
        );
}


int lcdspi_disp_bmp_file(lcdspi_t *lcdspi, int x, int y, const char *filename) {
    #define BMP_HEADER_SIZE 0x8a
    char BmpHeader[BMP_HEADER_SIZE];
    int ofs, wx, wy, depth, lineBytes, bufSize;
    int bitmapDy = 1;

    mp_obj_t file = mf_open(filename);
    uint32_t readed;
    readed = mf_read(file, (void *)BmpHeader, (mp_uint_t)BMP_HEADER_SIZE);
    if (readed != BMP_HEADER_SIZE) {
        mf_close(file);
        return -1;
    }
    ofs = BYTEARRAY4_TO_INT((uint8_t *)&BmpHeader[0x0a]);
    wx = BYTEARRAY4_TO_INT((uint8_t *)&BmpHeader[0x12]);
    wy = BYTEARRAY4_TO_INT((uint8_t *)&BmpHeader[0x16]);
    depth = BYTEARRAY2_TO_INT((uint8_t *)&BmpHeader[0x1c]);
    lineBytes = wx * depth / 8;
    bufSize = lineBytes * bitmapDy;
    uint8_t *bitmapOneLine = (uint8_t *)m_malloc(bufSize);
    if (!bitmapOneLine) {
        mf_close(file);
        return -1;
    }
    mf_seek(file, (uint32_t)ofs);
    if (depth == 16) {
        for (int ty = wy - 1 - bitmapDy; ty >= 0; ty -= bitmapDy) {
            readed = mf_read(file, (void *)bitmapOneLine, (uint32_t)bufSize);
            if (readed != bufSize) {
                mf_close(file);
                return -1;
            }
            for (int i = 0; i < wx; i++) {
                uint8_t l = bitmapOneLine[i * 2];
                uint8_t h = bitmapOneLine[i * 2 + 1];
                bitmapOneLine[i * 2] = h;
                bitmapOneLine[i * 2 + 1] = l;
            }
            lcdspi_bitbltex565(lcdspi, x, y + ty, wx, bitmapDy, (uint16_t *)bitmapOneLine);
        }
    } else if (depth == 24) {
        for (int ty = wy - 1 - bitmapDy; ty >= 0; ty -= bitmapDy) {
            readed = mf_read(file, (void *)bitmapOneLine, (uint32_t)bufSize);
            if (readed != bufSize) {
                mf_close(file);
                return -1;
            }
            for (int i = 0; i < wx; i++) {
                uint16_t b = (uint16_t)bitmapOneLine[i * 3];
                uint16_t g = (uint16_t)bitmapOneLine[i * 3 + 1];
                uint16_t r = (uint16_t)bitmapOneLine[i * 3 + 2];
                uint16_t v = (b >> 3) + ((g >> 2) << 5) + ((r >> 3) << 11);
                bitmapOneLine[i * 2] = (uint8_t)(v);
                bitmapOneLine[i * 2 + 1] = (uint8_t)(v >> 8);
            }
            lcdspi_bitbltex565(lcdspi, x, y + ty, wx, bitmapDy, (uint16_t *)bitmapOneLine);
        }
    }
    if (bitmapOneLine) {
        m_free(bitmapOneLine);
        mf_close(file);
    }
    return 1;
}

#endif
