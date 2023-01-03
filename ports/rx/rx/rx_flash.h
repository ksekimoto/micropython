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

#ifndef RX_RX_FLASH_H_
#define RX_RX_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef unsigned char CHIP_WORD;

#define FLASH_SECTION   __attribute__((section("FLASH_OP")))

#if defined(RX63N)
void fcu_Interrupt_Disable(void) FLASH_SECTION;
void fcu_Reset(void) FLASH_SECTION;
bool fcu_Transition_RomRead_Mode(volatile unsigned char *) FLASH_SECTION;
bool fcu_Transition_RomPE_Mode(unsigned int flash_addr) FLASH_SECTION;
bool fcu_Transfer_Firmware(volatile unsigned char *) FLASH_SECTION;
bool fcu_Notify_Peripheral_Clock(volatile unsigned char *) FLASH_SECTION;
bool fcu_Erase(volatile unsigned char *) FLASH_SECTION;
bool fcu_Write(volatile unsigned char *, unsigned short *, unsigned short *) FLASH_SECTION;
#endif

void *lmemset(void *dst, int c, size_t len) FLASH_SECTION;
void *lmemcpy(void *dst, const void *src, size_t len) FLASH_SECTION;
int lmemcmp(const void *p1, const void *p2, size_t len) FLASH_SECTION;

uint32_t sector_size(uint32_t addr) FLASH_SECTION;
uint32_t sector_start(uint32_t addr) FLASH_SECTION;
uint32_t sector_index(uint32_t addr) FLASH_SECTION;

bool internal_flash_read(void *context, unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff) FLASH_SECTION;
bool internal_flash_write(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite) FLASH_SECTION;
bool internal_flash_writex(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite, bool fIncrementDataPtr) FLASH_SECTION;
bool internal_flash_memset(unsigned char *addr, uint8_t Data, uint32_t NumBytes) FLASH_SECTION;
bool internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength) FLASH_SECTION;
bool internal_flash_eraseblock(unsigned char *addr) FLASH_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* RX_RX_FLASH_H_ */
