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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "py/runtime.h"
#include "py/mphal.h"

#if MICROPY_HW_ENABLE_LCDSPI

#if defined(_WIN32)
#include "ff_wrap.h"
#else
#include "mpy_file.h"
#endif
#include "jpeg_src.h"
#include "jpeg_debug.h"

void jpeg_src_set_file(jpeg_src_t *jpeg_src, const char *filename) {
    jpeg_src->type = JPEG_SRC_FILE;
    jpeg_src->filename = filename;
}

void jpeg_src_set_mem(jpeg_src_t *jpeg_src, const uint8_t *buf, uint32_t size) {
    jpeg_src->type = JPEG_SRC_MEM;
    jpeg_src->mem_size = size;
    jpeg_src->mem_buf = (uint8_t *)buf;
}

bool jpeg_src_open(jpeg_src_t *jpeg_src) {
    g_jpeg_src = jpeg_src;
    if (jpeg_src->type == JPEG_SRC_FILE) {
        #if defined(_WIN32)
        FRESULT res;
        res = f_open(&jpeg_src->fp, (const char *)jpeg_src->filename, "rb");
        if (res != FR_OK) {
            #if defined(DEBUG_JPEG_SRC)
            debug_printf("File can't be opened %s\r\n", jpeg_src->filename);
            #endif
            return false;
        }
        #else
        mp_obj_t file = mf_open((const char *)jpeg_src->filename);
        jpeg_src->fil = file;
        #endif
        jpeg_src->file_ofs = 0;
        #if defined(_WIN32)
        jpeg_src->file_size = (uint32_t)fsize(jpeg_src->fp);
        #else
        jpeg_src->file_size = (uint32_t)mf_size(jpeg_src->fil);
        #endif
        #if defined(DEBUG_JPEG_SRC)
        debug_printf("FileSize:%d\r\n", jpeg_src->file_size);
        #endif
    }
    return true;
}

void jpeg_src_close(jpeg_src_t *jpeg_src) {
    if (jpeg_src->type == JPEG_SRC_FILE) {
        #if defined(_WIN32)
        f_close(jpeg_src->fp);
        #else
        mf_close(jpeg_src->fil);
        #endif
        #if defined(DEBUG_JPEG_SRC)
        debug_printf("File closed\r\n");
        #endif
    }
}

void jpeg_src_read(jpeg_src_t *jpeg_src, void *pBuf, uint32_t n, uint32_t *readed) {
    uint32_t size = n;
    if (jpeg_src->type == JPEG_SRC_FILE) {
        #if defined(_WIN32)
        f_read(jpeg_src->fp, (void *)pBuf, (uint32_t)n, (uint32_t *)&size);
        #else
        size = mf_read(jpeg_src->fil, (void *)pBuf, (uint32_t)n);
        #endif
    } else {
        if ((uint32_t)n > (jpeg_src->mem_size - jpeg_src->mem_ofs)) {
            size = jpeg_src->mem_size - jpeg_src->mem_ofs;
        }
        char *p = ((char *)jpeg_src->mem_buf) + jpeg_src->mem_ofs;
        memcpy((void *)pBuf, (const void *)p, (size_t)size);
        jpeg_src->mem_ofs += size;
    }
    *readed = (uint32_t)size;
    #if defined(DEBUG_JPEG_SRC)
    if (size != n) {
        debug_printf("req size: %d ret size %d\r\n", n, size);
    }
    #endif
}

#endif
