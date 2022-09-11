/*
 * Copyright (c) 2021, Kentaro Sekimoto
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
#include "py/reader.h"
#include "py/stream.h"
#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "mpy_file.h"

mp_obj_t mf_open(const char *filename) {
    mp_obj_t args[2] = {
        mp_obj_new_str(filename, strlen(filename)),
        MP_OBJ_NEW_QSTR(MP_QSTR_rb),
    };
    return mp_vfs_open(MP_ARRAY_SIZE(args), &args[0], (mp_map_t *)&mp_const_empty_map);
}

uint32_t mf_read(mp_obj_t file, void *buf, uint32_t size) {
    uint32_t readed;
    int errcode;
    readed = (uint32_t)mp_stream_rw(file, (void *)buf, (mp_uint_t)size, &errcode, MP_STREAM_RW_READ | MP_STREAM_RW_ONCE);
    if (errcode != 0) {
        mp_stream_close(file);
        mp_raise_OSError(errcode);
    }
    return readed;
}

uint32_t mf_write(mp_obj_t file, void *buf, uint32_t size) {
    uint32_t written;
    int errcode;
    written = (uint32_t)mp_stream_rw(file, (void *)buf, (mp_uint_t)size, &errcode, MP_STREAM_RW_WRITE | MP_STREAM_RW_ONCE);
    if (errcode != 0) {
        mp_stream_close(file);
        mp_raise_OSError(errcode);
    }
    return written;
}

static uint32_t mf_seek_sub(mp_obj_t file, uint32_t ofs, int where) {
    struct mp_stream_seek_t seek_s;
    seek_s.offset = (mp_off_t)ofs;
    seek_s.whence = where;
    const mp_stream_p_t *stream_p = mp_get_stream(file);
    int error;
    mp_uint_t res = stream_p->ioctl(file, MP_STREAM_SEEK, (mp_uint_t)(uintptr_t)&seek_s, &error);
    if (res == MP_STREAM_ERROR) {
        mp_raise_OSError(error);
    }
    return (uint32_t)seek_s.offset;
}

uint32_t mf_seek(mp_obj_t file, uint32_t ofs) {
    return mf_seek_sub(file, ofs, SEEK_SET);
}

uint32_t mf_size(mp_obj_t file) {
    uint32_t cur = mf_seek_sub(file, 0, SEEK_CUR);
    uint32_t size = mf_seek_sub(file, 0, SEEK_END);
    mf_seek_sub(file, cur, SEEK_SET);
    return size;
}

void mf_close(mp_obj_t file) {
    mp_stream_close(file);
}
