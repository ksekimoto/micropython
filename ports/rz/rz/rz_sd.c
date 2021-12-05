/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
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
#include "iodefine.h"
#include "rz_gpio.h"
#include "ff.h"
#include "diskio.h"
#include "rz_sd.h"

#if defined(USE_DBG_PRINT)
// #define DEBUG_SD
#endif

// sd_util.c
int32_t sd_get_size(int32_t sd_port, uint32_t *user, uint32_t *protect);

#ifndef SD_SECTOR_SIZE
#define SD_SECTOR_SIZE 512
#endif

DSTATUS sd_disk_initialize(BYTE pdrv);
DSTATUS sd_disk_status(BYTE pdrv);
DRESULT sd_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
DRESULT sd_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);

static BYTE pdrv = 1;

void rz_sdcard_init(void) {
    #if defined(DEBUG_SD)
    DSTATUS ret = sd_disk_initialize(pdrv);
    debug_printf("SDI:%d\r\n", ret);
    #else
    sd_disk_initialize(pdrv);
    #endif
}

bool rz_sdcard_is_present(void) {
    DSTATUS ret = sd_disk_status(pdrv);
    #if defined(DEBUG_SD)
    debug_printf("SDP:%d\r\n", ret);
    #endif
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool rz_sdcard_power_on(void) {
    if (rz_sdcard_is_present()) {
        return true;
    } else {
        return false;
    }
}

void rz_sdcard_power_off(void) {
}

uint64_t rz_sdcard_get_capacity_in_bytes(void) {
    uint32_t size = 0;
    sd_get_size(pdrv, &size, NULL);
    #if defined(DEBUG_SD)
    debug_printf("SDS:%lx\r\n", (uint64_t)(size * SD_SECTOR_SIZE));
    #endif
    return (uint64_t)(size * SD_SECTOR_SIZE);
}

// static uint8_t nc_buf[512] __attribute__ ((section("NC_BSS"), aligned(32)));
uint32_t rz_sdcard_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    DRESULT ret = sd_disk_read(pdrv, (BYTE *)dest, (DWORD)block_num, (UINT)num_blocks);
    #if defined(DEBUG_SD)
    // debug_printf("SDR:%x:%x:%0x\r\n", block_num, num_blocks, dest);
    debug_printf("SDR:%x:%x:%x\r\n", block_num, num_blocks, ret);
    #endif
    return (uint32_t)ret;
}

uint32_t rz_sdcard_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    DRESULT ret = sd_disk_write(pdrv, (const BYTE *)src, (DWORD)block_num, (UINT)num_blocks);
    #if defined(DEBUG_SD)
    // debug_printf("SDW:%x:%x:%0x\r\n", block_num, num_blocks, src);
    debug_printf("SDW:%x:%x:%x\r\n", block_num, num_blocks, ret);
    #endif
    return (uint32_t)ret;
}
