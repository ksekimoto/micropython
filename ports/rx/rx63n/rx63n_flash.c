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

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx63n_flash.h"

#define RX63N

////////////////////////////////////////////////////////////////////////////
// For Debugging
////////////////////////////////////////////////////////////////////////////
//#define DEBUG_FLASH
//#define DEBUG_FLASH_ERROR
//#define DEBUG_FLASH_SKIP
//#define DEBUG_FLASH_WriteX
//#define DEBUG_FLASH_EraseBlock
//#define DEBUG_FLASH_Read
//#define DEBUG_FLASH_Memset

////////////////////////////////////////////////////////////////////////////
// From Renesas sample
////////////////////////////////////////////////////////////////////////////

#if defined(RX63N)
#define ROM_AREA_0  (0x00F80000)
#define ROM_AREA_1  (0x00F00000)
#define ROM_AREA_2  (0x00E80000)
#define ROM_AREA_3  (0x00E00000)
#endif

#if defined(RX610) || defined(RX62T) || defined(RX62N)
#define ICLK_HZ (96000000)
#define PCLK_HZ (48000000)
#define FCLK_HZ (48000000)
#define FLASH_CLOCK_HZ PCLK_HZ
#define WAIT_MAX_ROM_WRITE \
    ((uint32_t)(12000 * (50.0/(FLASH_CLOCK_HZ/1000000)))*(ICLK_HZ/1000000))

#elif  defined(RX630) || defined(RX63N)
#define ICLK_HZ (96000000)
#define PCLK_HZ (48000000)
#define FCLK_HZ (48000000)
#define FLASH_CLOCK_HZ FCLK_HZ
#define WAIT_MAX_ROM_WRITE \
    ((uint32_t)(12000 * (50.0/(FLASH_CLOCK_HZ/1000000)))*(ICLK_HZ/1000000))
#endif

// fcu_Reset()
#define WAIT_TRESW2     2520        /* Standby time data for tRESW2 timeout control */
// fcu_Transition_RomRead_Mode()
#define WAIT_TE16K      7603200     /* Standby time data for tE16K x1.1 timeout control */
// fcu_Transfer_Firmware()
#define FCU_FIRM_TOP    0xFEFFE000  /* FCU firmware store area start address = FEFFE000h */
#define FCU_RAM_TOP     0x007F8000  /* FCU RAM start address = 007F8000h */
#define FCU_RAM_SIZE    0x2000      /* Size of FCU RAM = 8 Kbytes */
// fcu_Notify_Peripheral_Clock()
#define PCKA_48MHZ      0x0030      /* Periphera module clock(PCLK) = 48MHz */
#define WAIT_TPCKA      1636        /* Standby time data for tPCKA timeout control */
// fcu_Write()
#define WAIT_TP256      345600      /* Standby time data for tP256 x1.1 timeout control */

////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// From Renesas sample
////////////////////////////////////////////////////////////////////////////

void fcu_Interrupt_Disable(void) {
    FLASH.FRDYIE.BYTE &= ~(1 << 0);
#if 0
    FLASH.FAEINT.uint8_t &= ~(1 << 7);
    FLASH.FAEINT.uint8_t &= ~(1 << 4);
    FLASH.FAEINT.uint8_t &= ~(1 << 3);
    FLASH.FAEINT.uint8_t &= ~(1 << 1);
    FLASH.FAEINT.uint8_t &= ~(1 << 0);
#else
    FLASH.FAEINT.BYTE &= ~0x9b;
#endif
    ICU.IPR[0x01].BIT.IPR = 0;      // FIFERR IPR
    ICU.IER[0x02].BIT.IEN5 = 0;     // FIFERR IEN
    ICU.IPR[0x02].BIT.IPR = 0;      // FRDYI IPR
    ICU.IER[0x02].BIT.IEN7 = 0;     // FRDYI IEN
}

void fcu_Reset(void) {
    volatile unsigned long count;
    FLASH.FRESETR.WORD = 0xCC01;
    count = WAIT_TRESW2;
    while (count-- != 0) ;
    FLASH.FRESETR.WORD = 0xCC00;
}

bool fcu_Transition_RomRead_Mode(volatile unsigned char *command_addr) {
    unsigned long count;
    count = WAIT_TE16K;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            FLASH.FENTRYR.WORD = 0xAA00;
            while (0x0000 != FLASH.FENTRYR.WORD) ;
            FLASH.FWEPROR.BYTE = 0x02;
            return false;
        }
    }
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.ERSERR == 1)
    || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
        if (FLASH.FSTATR0.BIT.ILGLERR == 1) {
            if (FLASH.FASTAT.BYTE != 0x10) {
                FLASH.FASTAT.BYTE = 0x10;
            }
        }
        *command_addr = 0x50;
    }
    FLASH.FENTRYR.WORD = 0xAA00;
    while (0x0000 != FLASH.FENTRYR.WORD) ;
    FLASH.FWEPROR.BYTE = 0x02;
    return true;
}

bool fcu_Transition_RomPE_Mode(unsigned int flash_addr) {
    FLASH.FENTRYR.WORD = 0xAA00;
    while (0x0000 != FLASH.FENTRYR.WORD) ;
#if defined(RX62N)
    FLASH.FENTRYR.WORD = 0xAA01;
#elif defined(RX63N)
    if (flash_addr >= ROM_AREA_0) {
        /* Enter ROM PE mode for addresses 0xFFF80000 - 0xFFFFFFFF */
        FLASH.FENTRYR.WORD = 0xAA01;
    } else if ((flash_addr < ROM_AREA_0) && (flash_addr >= ROM_AREA_1)) {
        /* Enter ROM PE mode for addresses 0xFFF00000 - 0xFFF7FFFF */
        FLASH.FENTRYR.WORD = 0xAA02;
    } else if ((flash_addr < ROM_AREA_1) && (flash_addr >= ROM_AREA_2)) {
        /* Enter ROM PE mode for addresses 0xFFE80000 - 0xFFEFFFFF */
        FLASH.FENTRYR.WORD = 0xAA04;
    } else {
        /* Enter ROM PE mode for addresses 0xFFE00000 - 0xFFE7FFFF */
        FLASH.FENTRYR.WORD = 0xAA08;
    }
#endif
    FLASH.FWEPROR.BYTE = 0x01;
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.ERSERR == 1)
    || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
        return false;
    }
    if (FLASH.FSTATR1.BIT.FCUERR == 1) {
        return false;
    }
    return true;
}

bool fcu_Transfer_Firmware(volatile unsigned char *command_addr) {
    if (FLASH.FENTRYR.WORD != 0x0000) {
        if (!fcu_Transition_RomRead_Mode(command_addr))
            return false;
    }
    FLASH.FCURAME.WORD = 0xC401;
    memcpy((void *)FCU_RAM_TOP, (void *)FCU_FIRM_TOP, (unsigned long)FCU_RAM_SIZE);
    return true;
}

bool fcu_Notify_Peripheral_Clock(volatile unsigned char *command_addr) {
    unsigned long count;
    FLASH.PCKAR.WORD = PCKA_48MHZ;
    *command_addr = 0xE9;
    *command_addr = 0x03;
    *(volatile unsigned short *)command_addr = 0x0F0F;
    *(volatile unsigned short *)command_addr = 0x0F0F;
    *(volatile unsigned short *)command_addr = 0x0F0F;
    *command_addr = 0xD0;

    count = WAIT_TPCKA;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if (FLASH.FSTATR0.BIT.ILGLERR == 1)
        return false;
    return true;
}

bool fcu_Erase(volatile unsigned char *command_addr) {
    unsigned long count;
    //FLASH.FWEPROR.uint8_t = 0x01;
    FLASH.FPROTR.WORD = 0x5501;
    *command_addr = 0x20;
    *command_addr = 0xD0;
    count = WAIT_TE16K;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.ERSERR == 1)) {
        return false;
    }
    return true;
}

bool fcu_Write(volatile unsigned char *command_addr, unsigned short *flash_addr, unsigned short *buf_addr) {
    unsigned long cycle_cnt;
    unsigned long count;
    //FLASH.FWEPROR.uint8_t = 0x01;
    FLASH.FPROTR.WORD = 0x5501;
    *command_addr = 0xE8;
    *command_addr = 0x40;
    cycle_cnt = 4;
    while (cycle_cnt <= 67) {
        *flash_addr = *buf_addr;
        buf_addr ++;
        flash_addr ++;
        cycle_cnt++;
    }
    *command_addr = 0xD0;
    count = WAIT_TP256;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count --;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if ((FLASH.FSTATR0.BIT.ILGLERR == 1)
    || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
        return false;
    }
    return true;
}

#define REGION3_SECTOR_SIZE 0x10000     // 64K
#define REGION3_SECTOR_MAX  16
#define REGION2_SECTOR_SIZE 0x8000      // 32K
#define REGION2_SECTOR_MAX  16
#define REGION1_SECTOR_SIZE 0x4000      // 16K
#define REGION1_SECTOR_MAX  30
#define REGION0_SECTOR_SIZE 0x1000      // 4K
#define REGION0_SECTOR_MAX  6

//#define FLASH_BUF_SIZE 0x100
//#define FLASH_BUF_ADDR_MASK 0xffffff00
//#define FLASH_BUF_OFF_MASK  0x000000ff
#define FLASH_BUF_SIZE 0x80
#define FLASH_BUF_ADDR_MASK 0xffffff80
#define FLASH_BUF_OFF_MASK  0x0000007f

uint8_t fFWLoaded = 0;
uint8_t flash_buf[FLASH_BUF_SIZE]  __attribute__((aligned (2)));


static void wait(volatile int count) {
    while (count-- > 0) {
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
    }
}

void *lmemset(void *dst, int c, size_t len) {
    char *p;
    for (p = (char *)dst; len > 0; len--)
        *(p++) = c;
    return (void *)dst;
}

void *lmemcpy(void *dst, const void *src, size_t len) {
    char *d = (char *)dst;
    const char *s = (const char *)src;
    for (; len > 0; len--)
        *(d++) = *(s++);
    return (void *)dst;
}

int lmemcmp(const void *p1, const void *p2, size_t len) {
    unsigned char *a, *b;
    size_t i;

    a = (unsigned char *)p1;
    b = (unsigned char *)p2;
    for(i = 0; i < len; i++) {
        if (*a != *b) {
            return (*a < *b) ? -1 : 1;
        }
        a++;
        b++;
    }
    return (int)0;
}

uint32_t sector_size(uint32_t addr) {
    if (addr >= 0xFFFF8000)
        return REGION0_SECTOR_SIZE;
    else if (addr >= 0xFFF80000)
        return REGION1_SECTOR_SIZE;
    else if (addr >= 0xFFF00000)
        return REGION2_SECTOR_SIZE;
    else
        return REGION3_SECTOR_SIZE;
}

uint32_t sector_start(uint32_t addr) {
    if (addr >= 0xFFFF8000)
        return (addr & ~(REGION0_SECTOR_SIZE - 1));
    else if (addr >= 0xFFF80000)
        return (addr & ~(REGION1_SECTOR_SIZE - 1));
    else if (addr >= 0xFFF00000)
        return (addr & ~(REGION2_SECTOR_SIZE - 1));
    else
        return (addr & ~(REGION3_SECTOR_SIZE - 1));
}

uint32_t sector_index(uint32_t addr) {
    if (addr >= 0xFFFF8000)
        return (7 - ((addr - 0xFFFF8000) / REGION0_SECTOR_SIZE));
    else if (addr >= 0xFFF80000)
        return (37 - ((addr - 0xFFF80000) / REGION1_SECTOR_SIZE));
    else if (addr >= 0xFFF00000)
        return (53 - ((addr - 0xFFF0000) / REGION2_SECTOR_SIZE));
    else
        return (69 - ((addr - 0xFFE0000) / REGION3_SECTOR_SIZE));
}

bool internal_flash_read(void *context, unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff) {
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_Read)
    debug_printf("Read(addr=%x, num=%x, psec=%x)\r\n", addr, NumBytes, pSectorBuff);
#endif
    CHIP_WORD *startaddr = (CHIP_WORD *)addr;
    CHIP_WORD *endaddr = (CHIP_WORD *)(addr + NumBytes);
    while (startaddr < endaddr) {
        *pSectorBuff++ = *startaddr++;
    }
    return TRUE;
}

bool internal_flash_write(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite) {
    return internal_flash_writex(addr, NumBytes, pSectorBuff, ReadModifyWrite, TRUE);
}

bool internal_flash_writex(unsigned char *addr, uint32_t NumBytes, uint8_t *pSectorBuff, bool ReadModifyWrite, bool fIncrementDataPtr) {
    __asm__ __volatile__ ("clrpsw i");
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    //debug_printf("WriteX(addr=%x, num=%x, psec=%x)\r\n", addr, NumBytes, pSectorBuff);
    //debug_printf("WriteX(addr=%x, num=%x)\r\n", addr, NumBytes);
    debug_printf("FLW:%x,%x,", addr, NumBytes);
#endif
    uint32_t error_code = 0;
#ifndef DEBUG_FLASH_SKIP
    bool flag;
    unsigned char *command_addr = (unsigned char *)((uint32_t)addr & 0x00FFFFFF);
    //FLASH_BEGIN_PROGRAMMING_FAST() ;
    uint32_t sector, count;
    uint32_t startaddr  = (uint32_t)addr & FLASH_BUF_ADDR_MASK;
    uint32_t offset = (uint32_t)addr & FLASH_BUF_OFF_MASK;
    uint32_t endaddr = (uint32_t)addr + NumBytes;
    while (startaddr < endaddr) {
        // copy from dst rom addr to flash buffer to keep current data
        memcpy(flash_buf, (void *)startaddr, FLASH_BUF_SIZE);
        //memset(flash_buf, 0xff, FLASH_BUF_SIZE);
        if (NumBytes + offset > FLASH_BUF_SIZE) {
            count = FLASH_BUF_SIZE - offset;
            NumBytes -= count;
        } else
            count = NumBytes;
        // overwrite data from src addr to flash buffer
        if (fIncrementDataPtr)
            memcpy(flash_buf + offset, pSectorBuff, count);
        else
            memset(flash_buf + offset, (int)*pSectorBuff, count);
        command_addr = (unsigned char *)((uint32_t)startaddr & 0x00FFFFFF);;
        unsigned short *flash_addr  = (unsigned short *)((uint32_t)startaddr & 0x00FFFFFF);
        unsigned short *buf_addr = (unsigned short *)&flash_buf[0];
        //wait(100000);
        //FLASH_BEGIN_PROGRAMMING_FAST() ;
        //__asm__ __volatile__ ("clrpsw i");
        fcu_Interrupt_Disable();
        flag = fcu_Transfer_Firmware(command_addr);
        if (!flag) {
            error_code = 1;
            goto WriteX_exit;
        }
        flag = fcu_Transition_RomPE_Mode((unsigned int)command_addr);
        if (!flag) {
            error_code = 2;
            goto WriteX_exit;
        }
        flag = fcu_Notify_Peripheral_Clock(command_addr);
        if (!flag) {
            error_code = 3;
            flag =  fcu_Transition_RomRead_Mode(command_addr);
            goto WriteX_exit;
        }
        flag = fcu_Write(command_addr, flash_addr, buf_addr);
        if (!flag) {
            error_code = 4;
            flag =  fcu_Transition_RomRead_Mode(command_addr);
            goto WriteX_exit;
        }
        flag =  fcu_Transition_RomRead_Mode(command_addr);
        if (!flag) {
            error_code = 5;
            goto WriteX_exit;
        }
        //FLASH_END_PROGRAMMING_FAST("", addr);
        //__asm__ __volatile__ ("setpsw i");
        if (fIncrementDataPtr) {
            flag= (memcmp((void *)(startaddr+offset), flash_buf+offset, count) == 0);
            if (!flag) {
                error_code = 6;
                break;
            }
        }
        offset = 0;
        startaddr += FLASH_BUF_SIZE;
        pSectorBuff += count;
    }
    //FLASH_END_PROGRAMMING_FAST("", addr);
WriteX_exit:
    __asm__ __volatile__ ("setpsw i");
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    //debug_printf("WriteX() error_code=%x\r\n", error_code);
    debug_printf("%x\r\n", error_code);
#endif
#endif
    if (error_code == 0)
        return true;
#if defined(DEBUG_FLASH_ERROR)
    debug_printf("Flash Write Fail:%x\r\n", error_code);
#endif
    return false;
}

bool internal_flash_memset(unsigned char *addr, uint8_t Data, uint32_t NumBytes) {
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_Memset)
    debug_printf("Memset(addr=%x, num=%x, data=%x)\r\n", addr, NumBytes, Data);
#endif
    CHIP_WORD chipData;
    memset(&chipData, Data, sizeof(CHIP_WORD));
    return internal_flash_writex(addr, NumBytes, (uint8_t *)&chipData, TRUE, FALSE);
}

bool _internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength, bool debug) {
    if (debug) {
#if defined(DEBUG_FLASH)
        //debug_printf("IsBlockErased(addr=%x, len=%x)\r\n", addr, BlockLength);
        debug_printf("FLI:%x,%x,", addr, BlockLength);
#endif
    }
    bool flag = TRUE;
    CHIP_WORD *startaddr = (CHIP_WORD *)addr;
    CHIP_WORD *endaddr = (CHIP_WORD *)(addr + BlockLength);
    while (startaddr < endaddr) {
        if (*startaddr++ != (CHIP_WORD)0xFFFFFFFF) {
            flag = FALSE;
            break;
        }
    }
    if (debug) {
#if defined(DEBUG_FLASH)
        //debug_printf("IsEraseBlock() error_code=%x\r\n", (flag == TRUE)? 0:1);
        debug_printf("%x\r\n", (flag == TRUE)? 0:1);
#endif
    }
    return flag;
}

bool internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength) {
    return _internal_flash_isblockerased(addr, BlockLength, TRUE);
}

// erase one page
bool internal_flash_eraseblock(unsigned char *addr) {
    __asm__ __volatile__ ("clrpsw i");
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    //debug_printf("EraseBlock(addr=%x)\r\n", addr);
    debug_printf("FLE:%x,", addr);
#endif
    uint32_t error_code = 0;
    bool flag;
#ifndef DEBUG_FLASH_SKIP
    unsigned char *command_addr = (unsigned char *)((uint32_t)addr & 0x00FFFFFF);
    unsigned long block_size  = (unsigned long)sector_size((uint32_t)addr);

    //FLASH_BEGIN_PROGRAMMING_FAST() ;
    fcu_Interrupt_Disable();
    flag = fcu_Transfer_Firmware(command_addr);
    if (!flag) {
        error_code = 1;
        goto EraseBlock_exit;
    }
    flag = fcu_Transition_RomPE_Mode((unsigned int)command_addr);
    if (!flag) {
        error_code = 2;
        goto EraseBlock_exit;
    }
    flag = fcu_Notify_Peripheral_Clock(command_addr);
    if (!flag) {
        error_code = 3;
        flag =  fcu_Transition_RomRead_Mode(command_addr);
        goto EraseBlock_exit;
    }
    flag = fcu_Erase(command_addr);
    if (!flag) {
        error_code = 4;
        flag =  fcu_Transition_RomRead_Mode(command_addr);
        goto EraseBlock_exit;
    }
    flag =  fcu_Transition_RomRead_Mode(command_addr);
    if (!flag) {
        error_code = 5;
        goto EraseBlock_exit;
    }
    flag = _internal_flash_isblockerased(addr, block_size, false);
    if (!flag) {
        error_code = 6;
    }
    //FLASH_END_PROGRAMMING_FAST("", addr);
EraseBlock_exit:
    __asm__ __volatile__ ("setpsw i");
#if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    //debug_printf("EraseBlock() error_code=%x\r\n", error_code);
    debug_printf("%x\r\n", error_code);
#endif
#endif
    if (error_code == 0)
        return true;
#if defined(DEBUG_FLASH_ERROR)
    debug_printf("Flash Erase Fail:%x\r\n", error_code);
#endif
    return false;
}
