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

#ifndef LIB_JPEG_JPEG_SRC_H_
#define LIB_JPEG_JPEG_SRC_H_

#define JPEG_SRC_FILE 0
#define JPEG_SRC_MEM  1

#if defined(_WIN32)
#include <stdio.h>
#else
#include "mpy_file.h"
#endif

typedef struct {
    uint8_t type;
    const char *filename;
    #if defined(_WIN32)
    FILE *fp;
    #else
    void *fil;
    #endif
    uint32_t file_size;
    uint32_t file_ofs;
    uint32_t mem_size;
    uint8_t *mem_buf;
    uint32_t mem_ofs;
} jpeg_src_t;

void jpeg_src_set_file(jpeg_src_t *jpeg_src, const char *filename);
void jpeg_src_set_mem(jpeg_src_t *jpeg_src, const uint8_t *buf, uint32_t size);
bool jpeg_src_open(jpeg_src_t *jpeg_src);
void jpeg_src_close(jpeg_src_t *jpeg_src);
void jpeg_src_read(jpeg_src_t *jpeg_src, void *pBuf, uint32_t n, uint32_t *readed);

extern jpeg_src_t *g_jpeg_src;

#endif /* LIB_JPEG_JPEG_SRC_H_ */
