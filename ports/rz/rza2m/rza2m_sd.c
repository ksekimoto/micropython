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
#include "common.h"
#include "iodefine.h"
#include "rza2m_gpio.h"
#include "ff.h"
#include "diskio.h"
#include "rza2m_sd.h"

#ifndef SD_SECTOR_SIZE
#define SD_SECTOR_SIZE 512
#endif

static const sd_map_t map_sd_cd[] = {
//   pin      ch     func
    {P64,       1,  5},
    {P54,       1,  3},
    {PC6,       1,  4},
    {P50,       1,  4},
    {PIN_END,   NC, 0}
};

static const sd_map_t map_sd_wp[] = {
//   pin      ch     func
    {P97,       1,  5},
    {PC7,       1,  4},
    {P51,       1,  4},
    {P55,       1,  3},
    {PIN_END,   NC, 0}
};

DSTATUS sd_disk_initialize(BYTE pdrv);
DSTATUS sd_disk_status(BYTE pdrv);
DRESULT sd_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT sd_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);

static BYTE pdrv = 1;

void rza2m_sdcard_init(void) {
    sd_disk_initialize(pdrv);
}

bool rza2m_sdcard_is_present(void) {
    DSTATUS ret = sd_disk_status(pdrv);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool rza2m_sdcard_power_on(void) {
    if (rza2m_sdcard_is_present()) {
        return true;
    } else {
        return false;
    }
}

void rza2m_sdcard_power_off(void) {
}

uint64_t rza2m_sdcard_get_capacity_in_bytes(void) {
    uint32_t size = 0;
    sd_get_size(pdrv, &size, NULL);
    return (uint64_t)(size * SD_SECTOR_SIZE);
}

uint32_t rza2m_sdcard_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    DRESULT ret = sd_disk_read(pdrv, (BYTE *)dest, (DWORD)block_num, (UINT)num_blocks);
    return (uint32_t)ret;
}

uint32_t rza2m_sdcard_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    DRESULT ret = sd_disk_write(pdrv, (const BYTE *)src, (DWORD)block_num, (UINT)num_blocks);
    return (uint32_t)ret;
}
