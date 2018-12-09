/*
 * Copyright (c) 2018, Kentaro Sekimoto
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
#include "common.h"
#include "ff.h"
#include "sd.h"

#if MICROPY_HW_HAS_SDCARD
bool sd_exists(char *fn) {
    FRESULT res;
    FILINFO fno;
    res = f_stat(fatfs_sd, fn, &fno);
    return (res == FR_OK);
}

bool sd_remove(char *fn) {
    FRESULT res;
    res = f_unlink(fatfs_sd, fn);
    return (res == FR_OK);
}

bool sd_open(FIL *fp, char *fn, unsigned char mode) {
    FRESULT res;
    res = f_open(fatfs_sd, fp, fn, mode);
    return (res == FR_OK);
}

int sd_read_byte(FIL *fp) {
    unsigned char c;
    int len = 0;
    FRESULT res = f_read(fp, &c, 1, &len);
    if (len == 0) {
        return -1;
    }
    return (int)c;
}

int sd_read(FIL *fp, unsigned char *buf, int size) {
    int len;
    FRESULT res = f_read(fp, buf, size, &len);
    return len;
}

int sd_write_byte(FIL *fp, unsigned char c) {
    int len;
    FRESULT res = f_write(fp, &c, 1, &len);
    return len;
}

int sd_write(FIL *fp, unsigned char *buf, int size) {
    int len;
    FRESULT res = f_write(fp, buf, size, &len);
    return len;
}

void sd_seek(FIL *fp, unsigned long pos) {
    FSIZE_t ofs = (FSIZE_t)pos;
    FRESULT res = f_lseek(fp, ofs);
}

unsigned long sd_size(FIL *fp) {
    return (unsigned long)f_size(fp);
}

void sd_flush(FIL *fp) {
    f_sync(fp);
}

void sd_close(FIL *fp) {
    f_close(fp);
}
#endif



