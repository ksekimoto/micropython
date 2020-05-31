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
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "common.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "ff.h"

#include "jpeg.h"
#include "picojpeg.h"

#if MICROPY_PY_PYB_LCDSPI

#if defined(USE_DBG_PRINT)
#define DEBUG_JPEG
#endif

static FIL g_pInFile;
static uint32_t g_nInFileSize;
static uint32_t g_nInFileOfs;

void jpeg_init(jpeg_t *jpeg) {
    g_nInFileSize = 0;
    g_nInFileOfs = 0;
    jpeg->err = 0;
    jpeg->is_available = 0;
    jpeg->mcu_x = 0;
    jpeg->mcu_y = 0;
    jpeg->row_pitch = 0;
    jpeg->decoded_width = 0;
    jpeg->decoded_height = 0;
    jpeg->row_blocks_per_mcu = 0;
    jpeg->col_blocks_per_mcu = 0;
    jpeg->m_split = 1;
    jpeg->pImage = (uint8_t *)NULL;
    jpeg->comps = 0;
    jpeg->MCUSPerRow = 0;
    jpeg->MCUSPerCol = 0;
    jpeg->MCUx = 0;
    jpeg->MCUy = 0;
    jpeg->scanType = PJPG_GRAYSCALE;
}

void jpeg_deinit(jpeg_t *jpeg) {
    f_close(&g_pInFile);
#if defined(DEBUG_JPEG)
    debug_printf("File closed");
#endif
    if (jpeg->pImage != (uint8_t *)NULL) {
        free(jpeg->pImage);
        jpeg->pImage = (uint8_t *)NULL;
#if defined(DEBUG_JPEG)
        debug_printf("pImage deallocated");
#endif
    }
}

inline int width(jpeg_t *jpeg) {
    return (int)jpeg->image_info.m_width;
}
inline int height(jpeg_t *jpeg) {
    return (int)jpeg->image_info.m_height;
}
inline int MCUWidth(jpeg_t *jpeg) {
    return (int)jpeg->image_info.m_MCUWidth;
}
inline int MCUHeight(jpeg_t *jpeg) {
    return (int)jpeg->image_info.m_MCUHeight;
}

unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data) {
    int n;
    uint32_t readed;
    n = (int)min(g_nInFileSize - g_nInFileOfs, buf_size);
#if defined(DEBUG_JPEG)
    debug_printf("FileSize:%d\r\n", g_nInFileSize);
    debug_printf("FileOfs:%d\r\n", g_nInFileOfs);
#endif
    f_read(&g_pInFile, (void *)pBuf, (UINT)n, (UINT *)&readed);
    *pBytes_actually_read = (unsigned char)(n);
    g_nInFileOfs += n;
    return 0;
}

int jpeg_decode(jpeg_t *jpeg, char *filename, int split) {
    FRESULT res;
    fs_user_mount_t *vfs_fat;
    const char *p_out;
    jpeg->m_split = split;

    mp_vfs_mount_t *vfs = mp_vfs_lookup_path(filename, &p_out);
    if (vfs != MP_VFS_NONE && vfs != MP_VFS_ROOT) {
        vfs_fat = MP_OBJ_TO_PTR(vfs->obj);
    } else {
#if defined(DEBUG_JPEG)
        debug_printf("Cannot find user mount for %s\r\n", filename);
#endif
        return -1;
    }
    res = f_open(&vfs_fat->fatfs, &g_pInFile, filename, FA_READ);
    if (res != FR_OK) {
#if defined(DEBUG_JPEG)
        debug_printf("File can't be opened %s\r\n", filename);
#endif
        return -1;
    }
    g_nInFileOfs = 0;
    g_nInFileSize = f_size(&g_pInFile);
#if defined(DEBUG_JPEG)
    debug_printf("FileSize:%d\r\n", g_nInFileSize);
#endif
    jpeg->err = (int)pjpeg_decode_init(&jpeg->image_info, (pjpeg_need_bytes_callback_t)pjpeg_need_bytes_callback, (void *)NULL, jpeg->m_split);
    if (jpeg->err != 0) {
#if defined(DEBUG_JPEG)
        debug_printf("pjpeg_decode_init() NG %d\r\n", (int )jpeg->err);
#endif
        if (jpeg->err == PJPG_UNSUPPORTED_MODE) {
#if defined(DEBUG_JPEG)
            debug_printf("Progressive JPEG not supported.%d\r\n", (int )jpeg->err);
#endif
        }
        f_close(&g_pInFile);
#if defined(DEBUG_JPEG)
        debug_printf("File closed %s\r\n", filename);
#endif
        return -1;
    }
    int MCUWidth = jpeg->image_info.m_MCUWidth;
    int MCUHeight = jpeg->image_info.m_MCUHeight;
    int width = jpeg->image_info.m_width;
    int height = jpeg->image_info.m_height;
    jpeg->decoded_width =
        jpeg->m_split ? (jpeg->image_info.m_MCUSPerRow * MCUWidth) / 8 : width;
    jpeg->decoded_height =
        jpeg->m_split ? (jpeg->image_info.m_MCUSPerCol * MCUHeight) / 8 : height;
    jpeg->comps = jpeg->image_info.m_comps;
    jpeg->row_pitch = MCUWidth * jpeg->image_info.m_comps;
    jpeg->MCUSPerRow = jpeg->image_info.m_MCUSPerRow;
    jpeg->MCUSPerCol = jpeg->image_info.m_MCUSPerCol;
    jpeg->scanType = jpeg->image_info.m_scanType;
#if defined(DEBUG_JPEG)
    debug_printf("decoded_width: %d\r\n", jpeg->decoded_width);
    debug_printf("decoded_height: %d\r\n", jpeg->decoded_height);
    debug_printf("m_MCUWidth: %d\r\n", MCUWidth);
    debug_printf("m_MCUHeight: %d\r\n", MCUHeight);
    debug_printf("m_comps: %d\r\n", jpeg->image_info.m_comps);
    debug_printf("row_pitch: %d\r\n", jpeg->row_pitch);
    debug_printf("MCUSPerRow: %d\r\n", jpeg->MCUSPerRow);
    debug_printf("MCUSPerCol: %d \r\n", jpeg->MCUSPerCol);
    debug_printf("scanType: %d\r\n", jpeg->image_info.m_scanType);
#endif
    int ImageSize = MCUWidth * MCUHeight * jpeg->image_info.m_comps;
    jpeg->pImage = (uint8_t *)malloc(ImageSize);
    if (!jpeg->pImage) {
#if defined(DEBUG_JPEG)
        debug_printf("Memory allocation failed.\r\n");
#endif
        f_close(&g_pInFile);
#if defined(DEBUG_JPEG)
        debug_printf("File closed %s\r\n", filename);
#endif
        return -1;
    } else {
#if defined(DEBUG_JPEG)
        debug_printf("pImage allocated:%d\r\n", ImageSize);
#endif
    }
    memset(jpeg->pImage, 0, ImageSize);
    jpeg->row_blocks_per_mcu = MCUWidth >> 3;
    jpeg->col_blocks_per_mcu = MCUHeight >> 3;
    jpeg->is_available = 1;
    return jpeg_decode_mcu(jpeg);
}

int jpeg_decode_mcu(jpeg_t *jpeg) {
    jpeg->err = pjpeg_decode_mcu();
    if (jpeg->err != 0) {
        jpeg->is_available = 0;
        f_close(&g_pInFile);
#if defined(DEBUG_JPEG)
        debug_printf("File closed\r\n", 0);
#endif
        if (jpeg->err != PJPG_NO_MORE_BLOCKS) {
#if defined(DEBUG_JPEG)
            debug_printf("pjpeg_decode_mcu() failed with status %d\r\n", jpeg->err);
#endif
            if (jpeg->pImage != (uint8_t *)NULL) {
                free(jpeg->pImage);
                jpeg->pImage = (uint8_t *)NULL;
#if defined(DEBUG_JPEG)
                debug_printf("pImage deallocated\r\n");
#endif
            }
            return -1;
        }
    }
    return 1;
}

int jpeg_read(jpeg_t *jpeg) {
    int y, x;
    uint8_t *pDst_row;

    if (jpeg->is_available == 0)
        return 0;
    if (jpeg->mcu_y >= jpeg->image_info.m_MCUSPerCol) {
        f_close(&g_pInFile);
#if defined(DEBUG_JPEG)
        debug_printf("File closed\r\n");
#endif
        if (jpeg->pImage != (uint8_t *)NULL) {
            free(jpeg->pImage);
            jpeg->pImage = (uint8_t *)NULL;
#if defined(DEBUG_JPEG)
            debug_printf("pImage deallocated\r\n");
#endif
        }
        return 0;
    }
    if (jpeg->m_split) {
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
            uint32_t y, x;
            for (y = 0; y < jpeg->col_blocks_per_mcu; y++) {
                uint32_t src_ofs = (y * 128U);
                for (x = 0; x < jpeg->row_blocks_per_mcu; x++) {
                    pDst_row[0] = jpeg->image_info.m_pMCUBufR[src_ofs];
                    pDst_row[1] = jpeg->image_info.m_pMCUBufG[src_ofs];
                    pDst_row[2] = jpeg->image_info.m_pMCUBufB[src_ofs];
                    pDst_row += 3;
                    src_ofs += 64;
                }
                pDst_row += (jpeg->row_pitch - 3 * jpeg->row_blocks_per_mcu);
#if defined(DEBUG_JPEG)
                debug_printf("pDst_row ofs: %d\r\n", (int)(pDst_row - jpeg->pImage));
                debug_printf("(row_pitch - 3 * row_blocks_per_mcu): %d\r\n", (int)(jpeg->row_pitch - 3 * jpeg->row_blocks_per_mcu));
#endif
            }
        }
    } else {
        int MCUWidth = jpeg->image_info.m_MCUWidth;
        int MCUHeight = jpeg->image_info.m_MCUHeight;
        int width = jpeg->image_info.m_width;
        int height = jpeg->image_info.m_height;

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
                        for (bx = 0; bx < bx_limit; bx++)
                            *pDst++ = *pSrcR++;
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
    if (jpeg->mcu_x == jpeg->image_info.m_MCUSPerRow) {
        jpeg->mcu_x = 0;
        jpeg->mcu_y++;
    }
    if (jpeg_decode_mcu(jpeg) == -1)
        jpeg->is_available = 0;
    return 1;
}
#endif

