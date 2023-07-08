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

#ifndef PORTS_RZ_MBED_MBED_FLASH_H_
#define PORTS_RZ_MBED_MBED_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef unsigned char CHIP_WORD;

#if 0
#define FLASH_SECTION   __attribute__((section("RAM_CODE")))
#else
#define FLASH_SECTION
#endif

uint32_t mbed_sector_size(uint32_t addr) FLASH_SECTION;
uint32_t mbed_sector_start(uint32_t addr) FLASH_SECTION;
uint32_t mbed_sector_index(uint32_t addr) FLASH_SECTION;

bool mbed_internal_flash_read(void *context, unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff) FLASH_SECTION;
bool mbed_internal_flash_write(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite) FLASH_SECTION;
bool mbed_internal_flash_writex(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite, bool fIncrementDataPtr) FLASH_SECTION;
bool mbed_internal_flash_memset(unsigned char *addr, uint8_t Data, uint32_t NumBytes) FLASH_SECTION;
bool mbed_internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength) FLASH_SECTION;
bool mbed_internal_flash_eraseblock(unsigned char *addr) FLASH_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* PORTS_RZ_MBED_MBED_FLASH_H_ */
