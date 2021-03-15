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
#include "iodefine.h"
#include "rz_gpio.h"
#include "ff.h"
#include "diskio.h"
#include "rz_sd.h"

#if defined(USE_DBG_PRINT)
//#define DEBUG_SD
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

//static uint8_t nc_buf[512] __attribute__ ((section("NC_BSS"), aligned(32)));
uint32_t rz_sdcard_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    DRESULT ret = sd_disk_read(pdrv, (BYTE *)dest, (DWORD)block_num, (UINT)num_blocks);
#if defined(DEBUG_SD)
    //debug_printf("SDR:%x:%x:%0x\r\n", block_num, num_blocks, dest);
    debug_printf("SDR:%x:%x:%x\r\n", block_num, num_blocks, ret);
#endif
    return (uint32_t)ret;
}

uint32_t rz_sdcard_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    DRESULT ret = sd_disk_write(pdrv, (const BYTE *)src, (DWORD)block_num, (UINT)num_blocks);
#if defined(DEBUG_SD)
    //debug_printf("SDW:%x:%x:%0x\r\n", block_num, num_blocks, src);
    debug_printf("SDW:%x:%x:%x\r\n", block_num, num_blocks, ret);
#endif
    return (uint32_t)ret;
}
