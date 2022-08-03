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

/*
 * Porting from the following repositories.
 * https://github.com/richgel999/picojpeg.git
 * https://github.com/MakotoKurauchi/JPEGDecoder.git
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// #include "common.h"
#include "jpeg_src.h"
#include "jpeg.h"
#include "picojpeg.h"
#include "jpeg_debug.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#if defined(USE_DBG_PRINT)
#define DEBUG_JPEG
#endif

void *m_malloc(size_t n);
void m_free(void *p);

jpeg_src_t *g_jpeg_src;

void jpeg_init(jpeg_t *jpeg) {
    jpeg->err = 0;
    jpeg->is_available = false;
    jpeg->mcu_x = 0;
    jpeg->mcu_y = 0;
    jpeg->row_pitch = 0;
    jpeg->decoded_width = 0;
    jpeg->decoded_height = 0;
    jpeg->row_blocks_per_mcu = 0;
    jpeg->col_blocks_per_mcu = 0;
    jpeg->reduce = true;
    jpeg->pImage = (uint8_t *)NULL;
    jpeg->comps = 0;
    jpeg->MCUSPerRow = 0;
    jpeg->MCUSPerCol = 0;
    jpeg->MCUx = 0;
    jpeg->MCUy = 0;
}

void jpeg_deinit(jpeg_t *jpeg) {
    jpeg_src_close(g_jpeg_src);
    if (jpeg->pImage != (uint8_t *)NULL) {
        m_free(jpeg->pImage);
        jpeg->pImage = (uint8_t *)NULL;
        #if defined(DEBUG_JPEG)
        debug_printf("pImage deallocated\r\n");
        #endif
    }
}

unsigned char pjpeg_need_bytes_callback(unsigned char *pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data) {
    uint32_t readed;
    jpeg_src_read(g_jpeg_src, (void *)pBuf, (uint32_t)buf_size, (uint32_t *)&readed);
    *pBytes_actually_read = (unsigned char)readed;
    return 0;
}

bool jpeg_decode(jpeg_t *jpeg, bool reduce) {
    jpeg->reduce = reduce;
    int ret = jpeg_src_open(g_jpeg_src);
    if (ret < 0) {
        return false;
    }
    jpeg->err = (uint32_t)pjpeg_decode_init(&jpeg->image_info, (pjpeg_need_bytes_callback_t)pjpeg_need_bytes_callback, (void *)NULL, (unsigned char)jpeg->reduce);
    if (jpeg->err != 0) {
        #if defined(DEBUG_JPEG)
        debug_printf("pjpeg_decode_init() NG %d\r\n", (int)jpeg->err);
        #endif
        if (jpeg->err == PJPG_UNSUPPORTED_MODE) {
            #if defined(DEBUG_JPEG)
            debug_printf("Progressive JPEG not supported.%d\r\n", (int)jpeg->err);
            #endif
        }
        jpeg_src_close(g_jpeg_src);
        return false;
    }
    uint16_t MCUWidth = (uint16_t)jpeg->image_info.m_MCUWidth;
    uint16_t MCUHeight = (uint16_t)jpeg->image_info.m_MCUHeight;
    uint16_t width = (uint16_t)jpeg->image_info.m_width;
    uint16_t height = (uint16_t)jpeg->image_info.m_height;
    jpeg->decoded_width =
        jpeg->reduce ? (jpeg->image_info.m_MCUSPerRow * MCUWidth) / 8 : width;
    jpeg->decoded_height =
        jpeg->reduce ? (jpeg->image_info.m_MCUSPerCol * MCUHeight) / 8 : height;
    jpeg->comps = jpeg->image_info.m_comps;
    jpeg->row_pitch = (uint16_t)MCUWidth * jpeg->image_info.m_comps;
    jpeg->MCUSPerRow = (uint16_t)jpeg->image_info.m_MCUSPerRow;
    jpeg->MCUSPerCol = (uint16_t)jpeg->image_info.m_MCUSPerCol;
    #if defined(DEBUG_JPEG)
    debug_printf("decoded_width: %d\r\n", jpeg->decoded_width);
    debug_printf("decoded_height: %d\r\n", jpeg->decoded_height);
    debug_printf("m_MCUWidth: %d\r\n", MCUWidth);
    debug_printf("m_MCUHeight: %d\r\n", MCUHeight);
    debug_printf("m_comps: %d\r\n", jpeg->image_info.m_comps);
    debug_printf("row_pitch: %d\r\n", jpeg->row_pitch);
    debug_printf("MCUSPerRow: %d\r\n", jpeg->MCUSPerRow);
    debug_printf("MCUSPerCol: %d \r\n", jpeg->MCUSPerCol);
    #endif
    uint32_t ImageSize = (uint32_t)(MCUWidth * MCUHeight * jpeg->image_info.m_comps);
    jpeg->pImage = (uint8_t *)m_malloc((size_t)ImageSize);
    if (!jpeg->pImage) {
        #if defined(DEBUG_JPEG)
        debug_printf("Memory allocation failed.\r\n");
        #endif
        jpeg_src_close(g_jpeg_src);
        return false;
    } else {
        #if defined(DEBUG_JPEG)
        debug_printf("pImage allocated:%d\r\n", ImageSize);
        #endif
    }
    memset(jpeg->pImage, 0, ImageSize);
    jpeg->row_blocks_per_mcu = MCUWidth >> 3;
    jpeg->col_blocks_per_mcu = MCUHeight >> 3;
    jpeg->is_available = true;
    return jpeg_decode_mcu(jpeg);
}

bool jpeg_decode_mcu(jpeg_t *jpeg) {
    jpeg->err = pjpeg_decode_mcu();
    if (jpeg->err != 0) {
        jpeg->is_available = false;
        jpeg_src_close(g_jpeg_src);
        #if defined(DEBUG_JPEG)
        debug_printf("File closed\r\n");
        #endif
        if (jpeg->err != PJPG_NO_MORE_BLOCKS) {
            #if defined(DEBUG_JPEG)
            debug_printf("pjpeg_decode_mcu() failed with status %d\r\n", jpeg->err);
            #endif
            if (jpeg->pImage != (uint8_t *)NULL) {
                m_free(jpeg->pImage);
                jpeg->pImage = (uint8_t *)NULL;
                #if defined(DEBUG_JPEG)
                debug_printf("pImage deallocated\r\n");
                #endif
            }
            return false;
        }
    }
    return true;
}

bool jpeg_read(jpeg_t *jpeg) {
    uint16_t y, x;
    uint8_t *pDst_row;

    if (!jpeg->is_available) {
        return false;
    }
    if (jpeg->mcu_y >= (uint32_t)jpeg->image_info.m_MCUSPerCol) {
        jpeg_src_close(g_jpeg_src);
        if (jpeg->pImage != (uint8_t *)NULL) {
            m_free(jpeg->pImage);
            jpeg->pImage = (uint8_t *)NULL;
            #if defined(DEBUG_JPEG)
            debug_printf("pImage deallocated\r\n");
            #endif
        }
        return false;
    }
    if (jpeg->reduce) {
        pDst_row = jpeg->pImage;
        #if defined(DEBUG_JPEG)
        debug_printf("col_blocks: %d\r\n", jpeg->col_blocks_per_mcu);
        debug_printf("row_blocks: %d\r\n", jpeg->row_blocks_per_mcu);
        debug_printf("pDst_row ofs: %d\r\n", (int)(pDst_row - jpeg->pImage));
        debug_printf("row_pitch: %d\r\n", jpeg->row_pitch);
        #endif
        if (jpeg->image_info.m_scanType == PJPG_GRAYSCALE) {
            *pDst_row = jpeg->image_info.m_pMCUBufR[0];
        } else {
            for (y = 0; y < jpeg->col_blocks_per_mcu; y++) {
                uint16_t src_ofs = (y * 128U);
                for (x = 0; x < jpeg->row_blocks_per_mcu; x++) {
                    pDst_row[0] = jpeg->image_info.m_pMCUBufR[src_ofs];
                    pDst_row[1] = jpeg->image_info.m_pMCUBufG[src_ofs];
                    pDst_row[2] = jpeg->image_info.m_pMCUBufB[src_ofs];
                    pDst_row += 3;
                    src_ofs += 64;
                }
                pDst_row += (uint32_t)(jpeg->row_pitch - 3 * jpeg->row_blocks_per_mcu);
                #if defined(DEBUG_JPEG)
                debug_printf("pDst_row ofs: %d\r\n", (int)(pDst_row - jpeg->pImage));
                debug_printf("(row_pitch - 3 * row_blocks_per_mcu): %d\r\n", (int)(jpeg->row_pitch - 3 * jpeg->row_blocks_per_mcu));
                #endif
            }
        }
    } else {
        uint16_t MCUWidth = jpeg->image_info.m_MCUWidth;
        uint16_t MCUHeight = jpeg->image_info.m_MCUHeight;
        uint16_t width = jpeg->image_info.m_width;
        uint16_t height = jpeg->image_info.m_height;

        pDst_row = jpeg->pImage;
        for (y = 0; y < MCUHeight; y += 8) {
            const int by_limit = min(8, height - (jpeg->mcu_y * MCUHeight + y));
            for (x = 0; x < MCUWidth; x += 8) {
                uint8_t *pDst_block = pDst_row + x * jpeg->image_info.m_comps;
                // Compute source byte offset of the block in the decoder's MCU buffer.
                uint32_t src_ofs = (x * 8U) + (y * 16U);
                const uint8_t *pSrcR = jpeg->image_info.m_pMCUBufR + src_ofs;
                const uint8_t *pSrcG = jpeg->image_info.m_pMCUBufG + src_ofs;
                const uint8_t *pSrcB = jpeg->image_info.m_pMCUBufB + src_ofs;
                const int bx_limit = min(8, width - (jpeg->mcu_x * MCUWidth + x));
                if (jpeg->image_info.m_scanType == PJPG_GRAYSCALE) {
                    int bx, by;
                    for (by = 0; by < by_limit; by++) {
                        uint8_t *pDst = pDst_block;
                        for (bx = 0; bx < bx_limit; bx++) {
                            *pDst++ = *pSrcR++;
                        }
                        pSrcR += (8 - bx_limit);
                        pDst_block += jpeg->row_pitch;
                    }
                } else {
                    int bx, by;
                    for (by = 0; by < by_limit; by++) {
                        uint8_t *pDst = pDst_block;
                        for (bx = 0; bx < bx_limit; bx++) {
                            pDst[0] = *pSrcR++;
                            pDst[1] = *pSrcG++;
                            pDst[2] = *pSrcB++;
                            pDst += 3;
                        }
                        pSrcR += (8 - bx_limit);
                        pSrcG += (8 - bx_limit);
                        pSrcB += (8 - bx_limit);
                        pDst_block += jpeg->row_pitch;
                    }
                }
            }
            pDst_row += (jpeg->row_pitch * 8);
        }
    }
    jpeg->MCUx = jpeg->mcu_x;
    jpeg->MCUy = jpeg->mcu_y;
    jpeg->mcu_x++;
    if (jpeg->mcu_x == (uint32_t)jpeg->image_info.m_MCUSPerRow) {
        jpeg->mcu_x = 0;
        jpeg->mcu_y++;
    }
    if (!jpeg_decode_mcu(jpeg)) {
        jpeg->is_available = false;
    }
    return true;
}
