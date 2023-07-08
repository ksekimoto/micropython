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

#include "mbed.h"
#include "flash_api.h"
#include "mbed_flash.h"
#include "rz_utils.h"

////////////////////////////////////////////////////////////////////////////
// For Debugging
////////////////////////////////////////////////////////////////////////////
#if defined(USE_DBG_PRINT)
#define DEBUG_FLASH
// #define DEBUG_FLASH_ERROR
// #define DEBUG_FLASH_SKIP
#define DEBUG_FLASH_Read
#define DEBUG_FLASH_WriteX
#define DEBUG_FLASH_EraseBlock
// #define DEBUG_FLASH_Read
// #define DEBUG_FLASH_Memset
#endif

#define RAM_CODE_SEC    __attribute__((section("RAM_CODE")))

void *lmemset(void *dst, int c, size_t len) {
    char *p;
    for (p = (char *)dst; len > 0; len--) {
        *(p++) = c;
    }
    return (void *)dst;
}

void *lmemcpy(void *dst, const void *src, size_t len) {
    char *d = (char *)dst;
    const char *s = (const char *)src;
    for (; len > 0; len--) {
        *(d++) = *(s++);
    }
    return (void *)dst;
}

int lmemcmp(const void *p1, const void *p2, size_t len) {
    unsigned char *a, *b;
    size_t i;

    a = (unsigned char *)p1;
    b = (unsigned char *)p2;
    for (i = 0; i < len; i++) {
        if (*a != *b) {
            return (*a < *b) ? -1 : 1;
        }
        a++;
        b++;
    }
    return (int)0;
}

static RAM_CODE_SEC flash_t MBED_FLASH;

uint32_t mbed_sector_size(uint32_t addr) {
    uint32_t ret;
    ret = flash_get_sector_size(&MBED_FLASH, addr);
    return ret;
}

uint32_t mbed_sector_start(uint32_t addr) {
    uint32_t ret;
    ret = addr & ~(mbed_sector_size(addr) - 1);
    return ret;
}

uint32_t mbed_sector_index(uint32_t addr) {
    uint32_t ret;
    uint32_t start;
    start = flash_get_start_address(&MBED_FLASH);
    if ((addr - start) > 0) {
        ret = (addr - start) / mbed_sector_size(addr);
    } else {
        ret = 0;
    }
    return ret;
}

bool mbed_internal_flash_read(void *context, unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff) {
    int32_t ret;
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_Read)
    // debug_printf("Read(addr=%x, num=%x, psec=%x)\r\n", addr, NumBytes, pSectorBuff);
    debug_printf("FLR:%x,%x,", addr, NumBytes);
    #endif
    ret = flash_read(&MBED_FLASH, (uint32_t)addr, pSectorBuff, NumBytes);
    return ret == 0;
}

bool mbed_internal_flash_write(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite) {
    int32_t ret;
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    // debug_printf("WriteX(addr=%x, num=%x, psec=%x)\r\n", addr, NumBytes, pSectorBuff);
    // debug_printf("WriteX(addr=%x, num=%x)\r\n", addr, NumBytes);
    debug_printf("FLW:%x,%x,", addr, NumBytes);
    #endif
    uint32_t state = rz_disable_irq();
    ret = flash_program_page(&MBED_FLASH, (uint32_t)addr, (const uint8_t *)pSectorBuff, NumBytes);
    rz_enable_irq(state);
    ret = lmemcmp((const void *)pSectorBuff, (const void *)addr, (size_t)NumBytes);
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    // debug_printf("EraseBlock() error_code=%x\r\n", error_code);
    debug_printf("%x\r\n", ret);
    #endif
    return ret == 0;
}

bool mbed_internal_flash_writex(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite, bool fIncrementDataPtr) {
    return true;
}

bool mbed_internal_flash_memset(unsigned char *addr, uint8_t Data, uint32_t NumBytes) {
    return true;
}

bool mbed_internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength) {
    return true;
}

bool mbed_internal_flash_eraseblock(unsigned char *addr) {
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    // debug_printf("EraseBlock(addr=%x)\r\n", addr);
    debug_printf("FLE:%x,", addr);
    #endif
    int32_t ret;
    uint32_t state = rz_disable_irq();
    ret = flash_erase_sector(&MBED_FLASH, (uint32_t)addr);
    rz_enable_irq(state);
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    // debug_printf("EraseBlock() error_code=%x\r\n", error_code);
    debug_printf("%x\r\n", ret);
    #endif
    return ret == 0;
}
