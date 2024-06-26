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

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "iodefine.h"
#include "interrupt_handlers.h"
#include "rx_flash.h"

#if defined(RX63N)
#define FLASH_TYPE_2
#endif

#if defined(RX64M)
#define FLASH_TYPE_3
#endif

#if defined(RX65N)
#define FLASH_TYPE_4
#endif

////////////////////////////////////////////////////////////////////////////
// For Debugging
////////////////////////////////////////////////////////////////////////////
// #define DEBUG_FLASH
// #define DEBUG_FLASH_ERROR
// #define DEBUG_FLASH_SKIP
// #define DEBUG_FLASH_WriteX
// #define DEBUG_FLASH_EraseBlock
// #define DEBUG_FLASH_Read
// #define DEBUG_FLASH_Memset

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
    ((uint32_t)(12000 * (50.0 / (FLASH_CLOCK_HZ / 1000000))) * (ICLK_HZ / 1000000))

#elif  defined(RX630) || defined(RX63N)
#define ICLK_HZ (96000000)
#define PCLK_HZ (48000000)
#define FCLK_HZ (48000000)
#define FLASH_CLOCK_HZ FCLK_HZ
#define WAIT_MAX_ROM_WRITE \
    ((uint32_t)(12000 * (50.0 / (FLASH_CLOCK_HZ / 1000000))) * (ICLK_HZ / 1000000))
#endif

#if defined(RX610) || defined(RX62T) || defined(RX62N) || defined(RX63N)
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

void FLASH_SECTION fcu_Interrupt_Disable(void) {
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
    while (count-- != 0) {
        ;
    }
    FLASH.FRESETR.WORD = 0xCC00;
}

bool fcu_Transition_RomRead_Mode(volatile unsigned char *command_addr) {
    unsigned long count;
    count = WAIT_TE16K;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count--;
        if (count == 0) {
            fcu_Reset();
            FLASH.FENTRYR.WORD = 0xAA00;
            while (0x0000 != FLASH.FENTRYR.WORD) {
                ;
            }
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
    while (0x0000 != FLASH.FENTRYR.WORD) {
        ;
    }
    FLASH.FWEPROR.BYTE = 0x02;
    return true;
}

bool FLASH_SECTION fcu_Transition_RomPE_Mode(unsigned int flash_addr) {
    FLASH.FENTRYR.WORD = 0xAA00;
    while (0x0000 != FLASH.FENTRYR.WORD) {
        ;
    }
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

bool FLASH_SECTION fcu_Transfer_Firmware(volatile unsigned char *command_addr) {
    if (FLASH.FENTRYR.WORD != 0x0000) {
        if (!fcu_Transition_RomRead_Mode(command_addr)) {
            return false;
        }
    }
    FLASH.FCURAME.WORD = 0xC401;
    lmemcpy((void *)FCU_RAM_TOP, (void *)FCU_FIRM_TOP, (unsigned long)FCU_RAM_SIZE);
    return true;
}

bool FLASH_SECTION fcu_Notify_Peripheral_Clock(volatile unsigned char *command_addr) {
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
        count--;
        if (count == 0) {
            fcu_Reset();
            return false;
        }
    }
    if (FLASH.FSTATR0.BIT.ILGLERR == 1) {
        return false;
    }
    return true;
}

bool FLASH_SECTION fcu_Erase(volatile unsigned char *command_addr) {
    unsigned long count;
    // FLASH.FWEPROR.uint8_t = 0x01;
    FLASH.FPROTR.WORD = 0x5501;
    *command_addr = 0x20;
    *command_addr = 0xD0;
    count = WAIT_TE16K;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count--;
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
    // FLASH.FWEPROR.uint8_t = 0x01;
    FLASH.FPROTR.WORD = 0x5501;
    *command_addr = 0xE8;
    *command_addr = 0x40;
    cycle_cnt = 4;
    while (cycle_cnt <= 67) {
        *flash_addr = *buf_addr;
        buf_addr++;
        flash_addr++;
        cycle_cnt++;
    }
    *command_addr = 0xD0;
    count = WAIT_TP256;
    while (FLASH.FSTATR0.BIT.FRDY == 0) {
        count--;
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

#endif

#if defined(RX65N)

#if defined(FLASH_TYPE_4)

/*FACI Commands*/
#define FLASH_FACI_CMD_PROGRAM              (0xE8)
#define FLASH_FACI_CMD_PROGRAM_CF           (0x80)
#define FLASH_FACI_CMD_PROGRAM_DF           (0x02)
#define FLASH_FACI_CMD_BLOCK_ERASE          (0x20)
#define FLASH_FACI_CMD_PE_SUSPEND           (0xB0)
#define FLASH_FACI_CMD_PE_RESUME            (0xD0)
#define FLASH_FACI_CMD_STATUS_CLEAR         (0x50)
#define FLASH_FACI_CMD_FORCED_STOP          (0xB3)
#define FLASH_FACI_CMD_BLANK_CHECK          (0x71)
#define FLASH_FACI_CMD_CONFIG_SET_1         (0x40)
#define FLASH_FACI_CMD_CONFIG_SET_2         (0x08)
#define FLASH_FACI_CMD_LOCK_BIT_PGM         (0x77)
#define FLASH_FACI_CMD_LOCK_BIT_READ        (0x71)
#define FLASH_FACI_CMD_FINAL                (0xD0)

#define PCKA_60MHZ  0x003C  /* Periphera module clock(PCLK) = 60MHz */
/* 185ms - assume 120MHz ICLK */
#define FLASH_FRDY_CMD_TIMEOUT  ((int32_t)(185000 * 120))

static volatile char *cmd_area = (volatile char *)(0x007E0000);

static bool FLASH_SECTION fcu_flash_init(void) {
    bool ret = true;
    /* Allow Access to the Flash registers */
    FLASH.FWEPROR.BYTE = 0x01;
    return ret;
}

static bool FLASH_SECTION fcu_flash_stop(void) {
    *cmd_area = (uint8_t)FLASH_FACI_CMD_FORCED_STOP;
    while (1 != FLASH.FSTATR.BIT.FRDY) {
        ; /* Wait for current operation to complete.*/
    }
    if (0 != FLASH.FASTAT.BIT.CMDLK) {
        return false;   // FLASH_ERR_CMD_LOCKED
    }
    return true;
}

#if 0
static bool FLASH_SECTION fcu_flash_reset(void) {
    if (FLASH.FASTAT.BIT.CFAE == 1) {
        FLASH.FASTAT.BIT.CFAE = 0;
    }
    /*Issue a forced stop */
    fcu_flash_stop();
    FLASH.FENTRYR.WORD = 0xAA00;    /* Transition to Read mode */
    while (FLASH.FENTRYR.WORD != 0x0000) {
        ;
    }
    return true;
}
#endif

static bool FLASH_SECTION fcu_flash_pe_mode_enter(void) {
    bool ret = true;
    FLASH.FENTRYR.WORD = 0xAA01;    /* Transition to CF P/E mode */
    int loop = 10;
    while (loop-- > 0) {
        if (FLASH.FENTRYR.WORD == 0x0001) {
            break;
        }
    }
    if (FLASH.FENTRYR.WORD != 0x0001) {
        ret = false;
    }
    return ret;
}

static bool FLASH_SECTION fcu_flash_wait_frdy(void) {
    bool flag = true;
    int count = FLASH_FRDY_CMD_TIMEOUT;
    while (1 != FLASH.FSTATR.BIT.FRDY) {
        if (count-- == 0) {
            fcu_flash_stop();
            flag = false;
        }
    }
    /* Check if Command Lock bit is set */
    if (flag == true) {
        if (0 != FLASH.FASTAT.BIT.CMDLK) {
            flag = false;
        }
    }
    return flag;
}

static bool FLASH_SECTION fcu_flash_pe_mode_exit(void) {
    bool flag = true;
    flag = fcu_flash_wait_frdy();
    FLASH.FENTRYR.WORD = 0xAA00;    /* Transition to Read mode */
    while (FLASH.FENTRYR.WORD != 0x0000) {
        ;
    }
    return flag;
}


bool FLASH_SECTION fcu_Notify_Peripheral_Clock(void) {
    bool flag = true;
    FLASH.FPCKAR.WORD = PCKA_60MHZ;
    if (FLASH.FASTAT.BIT.CMDLK == 1) {
        flag = fcu_flash_stop();
    }
    return flag;
}

static void FLASH_SECTION fcu_flash_int_disable(void) {
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

static bool FLASH_SECTION fcu_flash_erase(uint32_t block_address) {
    bool flag = true;
    FLASH.FCPSR.WORD = 0x0001; /* Set Erasure Priority Mode */
    FLASH.FSADDR.LONG = block_address;
    *cmd_area = (uint8_t)FLASH_FACI_CMD_BLOCK_ERASE;
    *cmd_area = (uint8_t)FLASH_FACI_CMD_FINAL;
    flag = fcu_flash_wait_frdy();
    return flag;
}

static bool FLASH_SECTION fcu_flash_write(uint16_t *buf_addr, uint16_t *flash_addr, uint32_t num_bytes) {
    bool flag = true;
    int cycle_cnt;
    int num = num_bytes >> 1;
    if (num > 0x40) {
        num = 0x40;
    }

    FLASH.FSADDR.LONG = (long)flash_addr;
    *cmd_area = (uint8_t)FLASH_FACI_CMD_PROGRAM;
    *cmd_area = (uint8_t)num;

    cycle_cnt = 0;
    while (cycle_cnt++ < num) {
        *(volatile uint16_t *)cmd_area = *(uint16_t *)buf_addr;
        while (FLASH.FSTATR.BIT.DBFULL == 1) {
            ;   // wait for fcu buffer to empty
        }
        buf_addr++;
    }
    *cmd_area = (uint8_t)FLASH_FACI_CMD_FINAL;
    flag = fcu_flash_wait_frdy();
    return flag;
}
#endif

#endif

#if defined(RX63N)
#define REGION3_SECTOR_SIZE 0x10000     // 64K
#define REGION3_SECTOR_MAX  16
#define REGION2_SECTOR_SIZE 0x8000      // 32K
#define REGION2_SECTOR_MAX  16
#define REGION1_SECTOR_SIZE 0x4000      // 16K
#define REGION1_SECTOR_MAX  30
#define REGION0_SECTOR_SIZE 0x1000      // 4K
#define REGION0_SECTOR_MAX  6

// #define FLASH_BUF_SIZE 0x100
// #define FLASH_BUF_ADDR_MASK 0xffffff00
// #define FLASH_BUF_OFF_MASK  0x000000ff
#define FLASH_BUF_SIZE 0x80
#define FLASH_BUF_ADDR_MASK 0xffffff80
#define FLASH_BUF_OFF_MASK  0x0000007f
#endif

#if defined(RX65N)
#define REGION1_SECTOR_SIZE 0x8000      // 32K
#define REGION1_SECTOR_MAX  62
#define REGION0_SECTOR_SIZE 0x2000      // 8K
#define REGION0_SECTOR_MAX  8

// #define FLASH_BUF_SIZE 0x100
// #define FLASH_BUF_ADDR_MASK 0xffffff00
// #define FLASH_BUF_OFF_MASK  0x000000ff
#define FLASH_BUF_SIZE 0x80             // For RX65N
#define FLASH_BUF_ADDR_MASK 0xffffff80
#define FLASH_BUF_OFF_MASK  0x0000007f
#endif

uint8_t flash_buf[FLASH_BUF_SIZE]  __attribute__((aligned(2)));

#if 0
static void FLASH_SECTION wait(volatile int count) {
    while (count-- > 0) {
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
        __asm__ __volatile__ ("nop");
    }
}
#endif

void *FLASH_SECTION lmemset(void *dst, int c, size_t len) {
    char *p;
    for (p = (char *)dst; len > 0; len--) {
        *(p++) = c;
    }
    return (void *)dst;
}

void *FLASH_SECTION lmemcpy(void *dst, const void *src, size_t len) {
    char *d = (char *)dst;
    const char *s = (const char *)src;
    for (; len > 0; len--) {
        *(d++) = *(s++);
    }
    return (void *)dst;
}

int FLASH_SECTION lmemcmp(const void *p1, const void *p2, size_t len) {
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

#if defined(RX63N)

uint32_t FLASH_SECTION sector_size(uint32_t addr) {
    if (addr >= 0xFFFF8000) {
        return REGION0_SECTOR_SIZE;
    } else if (addr >= 0xFFF80000) {
        return REGION1_SECTOR_SIZE;
    } else if (addr >= 0xFFF00000) {
        return REGION2_SECTOR_SIZE;
    } else {
        return REGION3_SECTOR_SIZE;
    }
}

uint32_t FLASH_SECTION sector_start(uint32_t addr) {
    if (addr >= 0xFFFF8000) {
        return addr & ~(REGION0_SECTOR_SIZE - 1);
    } else if (addr >= 0xFFF80000) {
        return addr & ~(REGION1_SECTOR_SIZE - 1);
    } else if (addr >= 0xFFF00000) {
        return addr & ~(REGION2_SECTOR_SIZE - 1);
    } else {
        return addr & ~(REGION3_SECTOR_SIZE - 1);
    }
}

uint32_t FLASH_SECTION sector_index(uint32_t addr) {
    if (addr >= 0xFFFF8000) {
        return 7 - ((addr - 0xFFFF8000) / REGION0_SECTOR_SIZE);
    } else if (addr >= 0xFFF80000) {
        return 37 - ((addr - 0xFFF80000) / REGION1_SECTOR_SIZE);
    } else if (addr >= 0xFFF00000) {
        return 53 - ((addr - 0xFFF0000) / REGION2_SECTOR_SIZE);
    } else {
        return 69 - ((addr - 0xFFE0000) / REGION3_SECTOR_SIZE);
    }
}

#elif defined(RX65N)

uint32_t FLASH_SECTION sector_size(uint32_t addr) {
    if (addr >= 0xFFFF0000) {
        return REGION0_SECTOR_SIZE;
    } else {
        return REGION1_SECTOR_SIZE;
    }
}

uint32_t FLASH_SECTION sector_start(uint32_t addr) {
    if (addr >= 0xFFFF0000) {
        return addr & ~(REGION0_SECTOR_SIZE - 1);
    } else {
        return addr & ~(REGION1_SECTOR_SIZE - 1);
    }
}

uint32_t FLASH_SECTION sector_index(uint32_t addr) {
    if (addr >= 0xFFFF0000) {
        return 7 - ((addr - 0xFFFF0000) / REGION0_SECTOR_SIZE);
    } else {
        return 69 - ((addr - 0xFFF00000) / REGION1_SECTOR_SIZE);
    }
}

#else
#error "RX MCU Series is not specified."
#endif

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
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    // debug_printf("WriteX(addr=%x, num=%x, psec=%x)\r\n", addr, NumBytes, pSectorBuff);
    // debug_printf("WriteX(addr=%x, num=%x)\r\n", addr, NumBytes);
    debug_printf("FLW:%x,%x,", addr, NumBytes);
    #endif
    __asm__ __volatile__ ("clrpsw i");
    uint32_t error_code = 0;
    #ifndef DEBUG_FLASH_SKIP
    bool flag;
    #if defined(RX63N)
    unsigned char *command_addr = (unsigned char *)((uint32_t)addr & 0x00FFFFFF);
    // FLASH_BEGIN_PROGRAMMING_FAST() ;
    #endif
    uint32_t count;
    uint32_t startaddr = (uint32_t)addr & FLASH_BUF_ADDR_MASK;
    uint32_t offset = (uint32_t)addr & FLASH_BUF_OFF_MASK;
    uint32_t endaddr = (uint32_t)addr + NumBytes;
    while (startaddr < endaddr) {
        // copy from dst rom addr to flash buffer to keep current data
        lmemcpy(flash_buf, (void *)startaddr, FLASH_BUF_SIZE);
        // memset(flash_buf, 0xff, FLASH_BUF_SIZE);
        if (NumBytes + offset > FLASH_BUF_SIZE) {
            count = FLASH_BUF_SIZE - offset;
            NumBytes -= count;
        } else {
            count = NumBytes;
        }
        // overwrite data from src addr to flash buffer
        if (fIncrementDataPtr) {
            lmemcpy(flash_buf + offset, pSectorBuff, count);
        } else {
            lmemset(flash_buf + offset, (int)*pSectorBuff, count);
        }
        #if defined(RX63N)
        command_addr = (unsigned char *)((uint32_t)startaddr & 0x00FFFFFF);
        ;
        unsigned short *flash_addr = (unsigned short *)((uint32_t)startaddr & 0x00FFFFFF);
        unsigned short *buf_addr = (unsigned short *)&flash_buf[0];
        // wait(100000);
        // FLASH_BEGIN_PROGRAMMING_FAST() ;
        // __asm__ __volatile__ ("clrpsw i");
        fcu_Interrupt_Disable();
        flag = fcu_Transfer_Firmware(command_addr);
        if (!flag) {
            error_code = 1;
            goto WriteX_exit;
        }
        flag = fcu_Transition_RomPE_Mode((unsigned int)command_addr);

        #elif defined(RX65N)

        unsigned short *flash_addr = (unsigned short *)((uint32_t)startaddr);
        unsigned short *buf_addr = (unsigned short *)&flash_buf[0];
        fcu_flash_int_disable();
        flag = fcu_flash_pe_mode_enter();

        #else
        #error "RX MCU Series is not specified."
        #endif
        if (!flag) {
            error_code = 2;
            goto WriteX_exit;
        }
        #if defined(RX63N)

        flag = fcu_Notify_Peripheral_Clock(command_addr);
        if (!flag) {
            error_code = 3;
            flag = fcu_Transition_RomRead_Mode(command_addr);
            goto WriteX_exit;
        }
        flag = fcu_Write(command_addr, flash_addr, buf_addr);
        if (!flag) {
            error_code = 4;
            flag = fcu_Transition_RomRead_Mode(command_addr);
            goto WriteX_exit;
        }
        flag = fcu_Transition_RomRead_Mode(command_addr);

        #elif defined(RX65N)

        flag = fcu_Notify_Peripheral_Clock();
        if (!flag) {
            error_code = 3;
            flag = fcu_flash_pe_mode_exit();
            goto WriteX_exit;
        }
        flag = fcu_flash_write(buf_addr, flash_addr, FLASH_BUF_SIZE);
        if (!flag) {
            error_code = 4;
            flag = fcu_flash_pe_mode_exit();
            goto WriteX_exit;
        }
        flag = fcu_flash_pe_mode_exit();

        #else
        #error "RX MCU Series is not specified."
        #endif
        if (!flag) {
            error_code = 5;
            goto WriteX_exit;
        }
        if (fIncrementDataPtr) {
            flag = (lmemcmp((void *)(startaddr + offset), flash_buf + offset, count) == 0);
            if (!flag) {
                error_code = 6;
                break;
            }
        }
        offset = 0;
        startaddr += FLASH_BUF_SIZE;
        pSectorBuff += count;
    }
WriteX_exit:
    #endif
    __asm__ __volatile__ ("setpsw i");
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_WriteX)
    // debug_printf("WriteX() error_code=%x\r\n", error_code);
    debug_printf("%x\r\n", error_code);
    #endif
    if (error_code == 0) {
        return true;
    }
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
    lmemset(&chipData, Data, sizeof(CHIP_WORD));
    return internal_flash_writex(addr, NumBytes, (uint8_t *)&chipData, TRUE, FALSE);
}

bool _internal_flash_isblockerased(unsigned char *addr, uint32_t BlockLength, bool debug) {
    if (debug) {
        #if defined(DEBUG_FLASH)
        // debug_printf("IsBlockErased(addr=%x, len=%x)\r\n", addr, BlockLength);
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
        // debug_printf("IsEraseBlock() error_code=%x\r\n", (flag == TRUE)? 0:1);
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
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    // debug_printf("EraseBlock(addr=%x)\r\n", addr);
    debug_printf("FLE:%x,", addr);
    #endif
    __asm__ __volatile__ ("clrpsw i");
    uint32_t error_code = 0;
    bool flag;
    #ifndef DEBUG_FLASH_SKIP
    #if defined(RX63N)

    unsigned char *command_addr = (unsigned char *)((uint32_t)addr & 0x00FFFFFF);
    unsigned long block_size = (unsigned long)sector_size((uint32_t)addr);

    // FLASH_BEGIN_PROGRAMMING_FAST() ;
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
        flag = fcu_Transition_RomRead_Mode(command_addr);
        goto EraseBlock_exit;
    }
    flag = fcu_Erase(command_addr);
    if (!flag) {
        error_code = 4;
        flag = fcu_Transition_RomRead_Mode(command_addr);
        goto EraseBlock_exit;
    }
    flag = fcu_Transition_RomRead_Mode(command_addr);

    #elif defined(RX65N)

    unsigned long block_size = (unsigned long)sector_size((uint32_t)addr);

    fcu_flash_int_disable();
    flag = fcu_flash_pe_mode_enter();
    if (!flag) {
        error_code = 2;
        goto EraseBlock_exit;
    }
    flag = fcu_Notify_Peripheral_Clock();
    if (!flag) {
        error_code = 3;
        flag = fcu_flash_pe_mode_exit();
        goto EraseBlock_exit;
    }
    flag = fcu_flash_erase((uint32_t)addr);
    if (!flag) {
        error_code = 4;
        flag = fcu_flash_pe_mode_exit();
        goto EraseBlock_exit;
    }
    flag = fcu_flash_pe_mode_exit();

    #else
    #error "RX MCU Series is not specified."
    #endif
    if (!flag) {
        error_code = 5;
        goto EraseBlock_exit;
    }
    flag = _internal_flash_isblockerased(addr, block_size, false);
    if (!flag) {
        error_code = 6;
    }
EraseBlock_exit:
    #endif
    __asm__ __volatile__ ("setpsw i");
    #if defined(DEBUG_FLASH) || defined(DEBUG_FLASH_EraseBlock)
    // debug_printf("EraseBlock() error_code=%x\r\n", error_code);
    debug_printf("%x\r\n", error_code);
    #endif
    if (error_code == 0) {
        return true;
    }
    #if defined(DEBUG_FLASH_ERROR)
    debug_printf("Flash Erase Fail:%x\r\n", error_code);
    #endif
    return false;
}

void internal_flash_init(void) {
    #if defined(RX63N)
    fcu_Reset();
    #elif defined(RX65N)
    fcu_flash_int_disable();
    fcu_flash_init();
    #else
    #error "RX MCU Series is not specified."
    #endif
}
