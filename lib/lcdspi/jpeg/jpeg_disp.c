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
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"

#if MICROPY_HW_ENABLE_LCDSPI

#include "jpeg_debug.h"
#include "lcdspi.h"
#include "jpeg_src.h"
#include "jpeg.h"
#include "jpeg_disp.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wconversion"
#endif

static bool lcdspi_disp_jpeg(lcdspi_t *lcdspi, uint32_t x, uint32_t y, bool split_disp) {
    jpeg_t m_jpeg;
    bool ret = true;
    bool reduce = false;    /* reduce mode = true -> not implemented */
    uint8_t *img;
    uint32_t cx, cy;
    uint32_t sx, sy;
    uint32_t decoded_width;
    uint32_t decoded_height;
    uint32_t alloc_size = 0;
    uint32_t MCUWidth;
    uint32_t MCUHeight;
    uint32_t width;
    uint32_t height;
    uint32_t lcd_wx = lcdspi->lcd->width;
    uint32_t lcd_wy = lcdspi->lcd->height;
    uint8_t *dispBuf = (uint8_t *)NULL;
    uint32_t tx, ty;

    jpeg_init(&m_jpeg);
    jpeg_decode(&m_jpeg, reduce);
    if (m_jpeg.err != 0) {
        #if defined(DEBUG_LCDSPI)
        debug_printf("jpeg decode error: %d\r\n", m_jpeg.err);
        #endif
        ret = false;
        goto lcdspi_disp_jpeg_exit;
    }
    decoded_width = m_jpeg.decoded_width;
    decoded_height = m_jpeg.decoded_height;
    MCUWidth = (uint32_t)m_jpeg.image_info.m_MCUWidth;
    MCUHeight = (uint32_t)m_jpeg.image_info.m_MCUHeight;
    width = (uint32_t)m_jpeg.image_info.m_width;
    height = (uint32_t)m_jpeg.image_info.m_height;
    if (split_disp) {
        alloc_size = MCUWidth * MCUHeight * sizeof(unsigned short);
    } else {
        alloc_size = width * height * sizeof(unsigned short);
    }
    dispBuf = (uint8_t *)m_malloc(alloc_size);
    if (!dispBuf) {
        #if defined(DEBUG_LCDSPI)
        debug_printf("dispBuf allocation failed.\r\n");
        #endif
        ret = false;
        goto lcdspi_disp_jpeg_exit;
    } else {
        #if defined(DEBUG_LCDSPI)
        debug_printf("dispBuf allocated: %d\r\n", alloc_size);
        #endif
    }
    uint32_t disp_width = 0;
    uint32_t disp_height = 0;
    uint32_t draw_width = decoded_width;
    uint32_t draw_height = decoded_height;
    if ((x + decoded_width) > lcd_wx) {
        draw_width = lcd_wx - x;
    }
    if ((y + decoded_height) > lcd_wy) {
        draw_height = lcd_wy - y;
    }
    while (jpeg_read((jpeg_t *)&m_jpeg)) {
        img = m_jpeg.pImage;
        if (!img) {
            break;
        }
        sx = m_jpeg.MCUx * MCUWidth;
        sy = m_jpeg.MCUy * MCUHeight;
        uint8_t *dst = dispBuf;
        if (split_disp) {
            memset(dispBuf, 0, alloc_size);
            disp_height = 0;
            for (ty = 0; ty < MCUHeight; ty++) {
                cy = sy + ty;
                if ((cy < height) && ((y + cy) < lcd_wy)) {
                    disp_width = 0;
                    for (tx = 0; tx < MCUWidth; tx++) {
                        cx = sx + tx;
                        if ((cx < width) && ((x + cx) < lcd_wx)) {
                            uint16_t val;
                            if (m_jpeg.comps == 1) {
                                /* gray scale */
                                val = (((img[0] >> 3) << 11)) | (((img[0] >> 2) << 5)) | ((img[0] >> 3));
                            } else {
                                val = (((img[0] >> 3) << 11)) | (((img[1] >> 2) << 5)) | ((img[2] >> 3));
                            }
                            *dst++ = (uint8_t)(val >> 8);
                            *dst++ = (uint8_t)(val & 0xff);
                            disp_width++;
                        }
                        img += m_jpeg.comps;
                    }
                    disp_height++;
                }
            }
            #if defined(DEBUG_LCDSPI)
            debug_printf("x: %d y: %d w: %d h: %d\r\n", x + sx, y + sy, disp_width, disp_height);
            #endif
            lcdspi_bitbltex565(lcdspi, x + sx, y + sy, disp_width, disp_height, (uint16_t *)dispBuf);
        } else {
            uint32_t ofs;
            for (ty = 0; ty < MCUHeight; ty++) {
                cy = sy + ty;
                if ((cy < height) && ((y + cy) < lcd_wy)) {
                    for (tx = 0; tx < MCUWidth; tx++) {
                        cx = sx + tx;
                        if ((cx < width) && ((x + cx) < lcd_wx)) {
                            uint16_t val;
                            if (m_jpeg.comps == 1) {
                                val = (((img[0] >> 3) << 11)) | (((img[0] >> 2) << 5)) | ((img[0] >> 3));
                            } else {
                                val = (((img[0] >> 3) << 11)) | (((img[1] >> 2) << 5)) | ((img[2] >> 3));
                            }
                            ofs = (uint32_t)((cy * draw_width + cx) * 2);
                            dispBuf[ofs] = (uint8_t)(val >> 8);
                            dispBuf[ofs + 1] = (uint8_t)(val & 0xff);
                        }
                        img += m_jpeg.comps;
                    }
                }
            }
            #if defined(DEBUG_LCDSPI)
            debug_printf("x: %d y: %d\r\n", x + sx, y + sy);
            #endif
        }
    }
    #if defined(DEBUG_LCDSPI)
    debug_printf("err=%d\r\n", m_jpeg.err);
    #endif
    if (!split_disp) {
        #if defined(DEBUG_LCDSPI)
        debug_printf("x: %d y: %d w: %d h: %d\r\n", x, y, draw_width, height);
        #endif
        lcdspi_bitbltex565(lcdspi, x, y, draw_width, draw_height, (uint16_t *)dispBuf);
    }
    if (dispBuf) {
        #if defined(DEBUG_LCDSPI)
        debug_printf("dispBuf deallocated\r\n");
        #endif
        m_free(dispBuf);
    }
lcdspi_disp_jpeg_exit:
    jpeg_deinit(&m_jpeg);
    return ret;
}

void jpeg_disp_file(lcdspi_t *lcdspi, uint32_t x, uint32_t y, const char *filename, bool split_disp) {
    jpeg_src_t jpeg_src;
    g_jpeg_src = &jpeg_src;
    jpeg_src_set_file((jpeg_src_t *)&jpeg_src, filename);
    lcdspi_disp_jpeg(lcdspi, x, y, split_disp);
}

void jpeg_disp_data(lcdspi_t *lcdspi, uint32_t x, uint32_t y, const uint32_t *data, uint32_t size, bool split_disp) {
    jpeg_src_t jpeg_src;
    g_jpeg_src = &jpeg_src;
    jpeg_src_set_mem((jpeg_src_t *)&jpeg_src, (uint8_t *)data, (uint32_t)size);
    lcdspi_disp_jpeg(lcdspi, x, y, split_disp);
}

#endif
