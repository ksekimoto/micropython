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

#ifndef RX63N_FLASH_H_
#define RX63N_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef unsigned char CHIP_WORD;

#define FLASH_SECTION   __attribute__((section ("FLASH_OP")))

void fcu_Interrupt_Disable(void) FLASH_SECTION;
void fcu_Reset(void) FLASH_SECTION;
bool fcu_Transition_RomRead_Mode(volatile unsigned char *) FLASH_SECTION;
bool fcu_Transition_RomPE_Mode(unsigned int flash_addr) FLASH_SECTION;
bool fcu_Transfer_Firmware(volatile unsigned char *) FLASH_SECTION;
bool fcu_Notify_Peripheral_Clock(volatile unsigned char *) FLASH_SECTION;
bool fcu_Erase(volatile unsigned char *) FLASH_SECTION;
bool fcu_Write(volatile unsigned char *, unsigned short *, unsigned short *) FLASH_SECTION;

void *lmemset(void *dst, int c, size_t len) FLASH_SECTION;
void *lmemcpy(void *dst, const void *src, size_t len) FLASH_SECTION;
int lmemcmp(const void *p1, const void *p2, size_t len) FLASH_SECTION;

uint32_t sector_size(uint32_t addr) FLASH_SECTION;
uint32_t sector_start(uint32_t addr) FLASH_SECTION;

bool internal_flash_read(void *context, unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff) FLASH_SECTION;
bool internal_flash_write(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite) FLASH_SECTION;
bool internal_flash_writex(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite, bool fIncrementDataPtr) FLASH_SECTION;
bool internal_flash_memset(unsigned char *addr, uint8_t Data, uint32_t NumBytes) FLASH_SECTION;
bool internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength) FLASH_SECTION;
bool internal_flash_eraseblock(unsigned char *addr) FLASH_SECTION;

#ifdef __cplusplus
}
#endif

#endif /* RX63N_FLASH_H_ */
