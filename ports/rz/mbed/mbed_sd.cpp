/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Kentaro Sekimoto
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

#include "stdbool.h"
#include "mbed.h"
#include "PinNames.h"
#include "SDHSBlockDevice.h"
#include "mbed_sd.h"

#define MBED_SD_CD_PIN  P6_4
#define MBED_SD_WP_PIN  NC
#define SDCARD_BLOCK_SIZE 512

static SDHSBlockDevice *mbed_sd_bd;
static const int sd_block_size = SDCARD_BLOCK_SIZE;

void mbed_sd_init(uint32_t cd, uint32_t wp) {
    mbed_sd_bd = new SDHSBlockDevice((PinName)cd, (PinName)wp);
    mbed_sd_bd->init();
}

void mbed_sd_deinit() {
    if (mbed_sd_bd) {
        mbed_sd_bd->deinit();
        delete mbed_sd_bd;
    }
}

void mbed_sdcard_init(void) {
    mbed_sd_init(MBED_SD_CD_PIN, MBED_SD_WP_PIN);
}

void mbed_sdcard_deinit(void) {
    mbed_sd_deinit();
}

bool mbed_sdcard_is_present(void) {
    return mbed_sd_bd->is_connect();
}

bool mbed_sdcard_power_on(void) {
    return true;
}

void mbed_sdcard_power_off(void) {

}

uint64_t mbed_sdcard_get_capacity_in_bytes(void) {
    return (uint64_t)mbed_sd_bd->size();
}

uint mbed_sdcard_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    int ret;
    ret = mbed_sd_bd->read((void *)dest, (bd_addr_t)sd_block_size * (bd_addr_t)block_num, (bd_addr_t)sd_block_size * (bd_size_t)num_blocks);
    if (ret > 0) {
        return 0;
    } else {
        return 1;
    }
}

uint mbed_sdcard_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    int ret;
    ret = mbed_sd_bd->program((const void *)src, (bd_addr_t)sd_block_size * (bd_addr_t)block_num, (bd_addr_t)sd_block_size * (bd_size_t)num_blocks);
    if (ret > 0) {
        return 0;
    } else {
        return 1;
    }
}
